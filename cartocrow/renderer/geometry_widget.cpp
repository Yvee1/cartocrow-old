/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "geometry_widget.h"
#include "cartocrow/renderer/geometry_renderer.h"

#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QSlider>
#include <QToolButton>
#include <QtGlobal>
#include <qpolygon.h>

namespace cartocrow {
namespace renderer {

GeometryWidget::GeometryWidget(GeometryPainting& painting) : m_painting(painting) {
	setMouseTracking(true);
	m_transform.scale(1, -1);

	m_zoomBar = new QToolBar(this);
	m_zoomOutButton = new QToolButton(m_zoomBar);
	m_zoomOutButton->setText("-");
	connect(m_zoomOutButton, &QToolButton::clicked, this, &GeometryWidget::zoomOut);
	m_zoomBar->addWidget(m_zoomOutButton);
	m_zoomSlider = new QSlider(m_zoomBar);
	m_zoomSlider->setOrientation(Qt::Horizontal);
	m_zoomSlider->setMinimumWidth(200);
	m_zoomSlider->setMaximumWidth(200);
	m_zoomSlider->setMinimum(0);
	m_zoomSlider->setMaximum(200);
	m_zoomSlider->setEnabled(false);
	m_zoomBar->addWidget(m_zoomSlider);
	m_zoomInButton = new QToolButton(m_zoomBar);
	m_zoomInButton->setText("+");
	connect(m_zoomInButton, &QToolButton::clicked, this, &GeometryWidget::zoomIn);
	m_zoomBar->addWidget(m_zoomInButton);
}

void GeometryWidget::resizeEvent(QResizeEvent* e) {
	QWidget::resizeEvent(e);
	QRect r = rect();
	QSize zoomBarSize = m_zoomBar->sizeHint();
	QRect zoomBarRect(QPoint{r.left(), r.bottom() - zoomBarSize.height()},
	                  QPoint{r.left() + zoomBarSize.width(), r.bottom()});
	m_zoomBar->setGeometry(zoomBarRect);
}

void GeometryWidget::paintEvent(QPaintEvent* event) {
	m_painter = std::make_unique<QPainter>(this);
	m_painter->setRenderHint(QPainter::Antialiasing);
	m_painter->fillRect(rect(), Qt::white);
	if (m_drawAxes) {
		drawAxes();
	}
	m_painting.paint(*this);
	m_painter->end();
}

void GeometryWidget::mouseMoveEvent(QMouseEvent* event) {
	Point converted = inverseConvertPoint(event->pos());
	auto x = static_cast<int>(converted.x() + 0.5);
	auto y = static_cast<int>(converted.y() + 0.5);
	m_mousePos = QPointF(x, y);

	if (m_dragging) {
		QPointF delta = event->pos() - m_previousMousePos;
		QTransform translation;
		translation.translate(delta.x(), delta.y());
		m_transform = m_transform * translation;
	}

	m_previousMousePos = event->pos();
	update();
}

void GeometryWidget::mousePressEvent(QMouseEvent* event) {
	m_dragging = true;
	setCursor(Qt::ClosedHandCursor);
	m_previousMousePos = event->pos();
	update();
}

void GeometryWidget::mouseReleaseEvent(QMouseEvent* event) {
	m_dragging = false;
	setCursor(Qt::ArrowCursor);
}

void GeometryWidget::leaveEvent(QEvent* event) {
	// TODO?
	update();
}

QSize GeometryWidget::sizeHint() const {
	return QSize(800, 450);
}

void GeometryWidget::wheelEvent(QWheelEvent* event) {
	if (event->angleDelta().isNull()) {
		return;
	}

	double delta = event->angleDelta().y();
	double factor = pow(2, delta / 240);

	// limit the zoom factor
	if (factor * m_transform.m11() > m_maxZoom) {
		factor = m_maxZoom / m_transform.m11();
	} else if (factor * m_transform.m11() < m_minZoom) {
		factor = m_minZoom / m_transform.m11();
	}

	QPointF mousePos = event->position();
	mousePos -= QPointF(width() / 2.0, height() / 2.0);
	QTransform transform;
	transform.translate(mousePos.x(), mousePos.y());
	transform.scale(factor, factor);
	transform.translate(-mousePos.x(), -mousePos.y());
	m_transform *= transform;

	updateZoomSlider();
	update();
}

QPointF GeometryWidget::convertPoint(Point p) const {
	QPointF mapped = m_transform.map(QPointF(p.x() + 0.5, p.y() + 0.5));
	return mapped + QPointF(width(), height()) / 2.0;
}

QRectF GeometryWidget::convertBox(Box b) const {
	QPointF topLeft = convertPoint(Point(b.xmin(), b.ymin()));
	QPointF bottomRight = convertPoint(Point(b.xmax(), b.ymax()));
	return QRectF(topLeft, bottomRight);
}

Point GeometryWidget::inverseConvertPoint(QPointF p) const {
	QPointF toMap = p - QPointF(width(), height()) / 2.0;
	QPointF transformed = m_transform.inverted().map(toMap) - QPointF(0.5, 0.5);
	return Point(transformed.x(), transformed.y());
}

Box GeometryWidget::inverseConvertBox(QRectF r) const {
	Point topLeft = inverseConvertPoint(r.topLeft());
	Point bottomRight = inverseConvertPoint(r.bottomRight());
	return Box(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

void GeometryWidget::drawAxes() {
	Box bounds = inverseConvertBox(rect());
	pushStyle();

	double tickScale = log10(m_transform.m11());
	double majorScale = pow(10, 2 - floor(tickScale));

	// minor grid lines
	double minorTint = tickScale - floor(tickScale);
	int minorColor = 255 - 64 * minorTint;
	setMode({});
	setStroke(Color{minorColor, minorColor, minorColor}, 1);
	for (int i = floor(bounds.xmin() / (majorScale / 10)); i <= bounds.xmax() / (majorScale / 10);
	     ++i) {
		draw(Segment(Point(i * majorScale / 10, bounds.ymin()),
		             Point(i * majorScale / 10, bounds.ymax())));
	}
	for (int i = floor(bounds.ymax() / (majorScale / 10)); i <= bounds.ymin() / (majorScale / 10);
	     ++i) {
		draw(Segment(Point(bounds.xmin(), i * majorScale / 10),
		             Point(bounds.xmax(), i * majorScale / 10)));
	}

	// major grid lines
	setStroke(Color{192, 192, 192}, 1);
	for (int i = floor(bounds.xmin() / majorScale); i <= bounds.xmax() / majorScale; ++i) {
		draw(Segment(Point(i * majorScale, bounds.ymin()), Point(i * majorScale, bounds.ymax())));
	}
	for (int i = floor(bounds.ymax() / majorScale); i <= bounds.ymin() / majorScale; ++i) {
		draw(Segment(Point(bounds.xmin(), i * majorScale), Point(bounds.xmax(), i * majorScale)));
	}

	// axes
	setStroke(Color{150, 150, 150}, 1.8);
	draw(Segment(Point(bounds.xmin(), 0), Point(bounds.xmax(), 0)));
	draw(Segment(Point(0, bounds.ymin()), Point(0, bounds.ymax())));

	// labels
	QPointF origin = convertPoint(Point(0, 0));
	m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
	                    Qt::AlignRight, "0");

	for (int i = floor(bounds.xmin() / majorScale); i <= bounds.xmax() / majorScale + 1; ++i) {
		if (i != 0) {
			origin = convertPoint(Point(i * majorScale, 0));
			if (origin.y() < 0) {
				origin.setY(0);
			} else if (origin.y() > rect().bottom() - 30) {
				origin.setY(rect().bottom() - 30);
			}
			m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
			                    Qt::AlignRight, QString::number(i * majorScale));
		}
	}
	QFontMetricsF metrics(m_painter->font());
	for (int i = floor(bounds.ymax() / majorScale); i <= bounds.ymin() / majorScale + 1; ++i) {
		if (i != 0) {
			origin = convertPoint(Point(0, i * majorScale));
			QString label = QString::number(i * majorScale);
			double length = metrics.width(label);
			if (origin.x() < length + 10) {
				origin.setX(length + 10);
			} else if (origin.x() > rect().right() - 0) {
				origin.setX(rect().right() - 0);
			}
			m_painter->drawText(QRectF(origin + QPointF{-100, 5}, origin + QPointF{-5, 100}),
			                    Qt::AlignRight, label);
		}
	}

	popStyle();
}

void GeometryWidget::updateZoomSlider() {
	double zoom = m_transform.m11();
	double fraction = log(zoom / m_minZoom) / log(m_maxZoom / m_minZoom);
	m_zoomSlider->setValue(fraction * 200);
}

void GeometryWidget::draw(const Point& p) {
	m_painter->setPen(Qt::NoPen);
	m_painter->setBrush(m_style.m_strokeColor);
	QPointF p2 = convertPoint(p);
	m_painter->drawEllipse(QRectF(p2.x() - 0.5 * m_style.m_pointSize,
	                              p2.y() - 0.5 * m_style.m_pointSize, m_style.m_pointSize,
	                              m_style.m_pointSize));
}

void GeometryWidget::draw(const Segment& s) {
	m_painter->setPen(QPen(m_style.m_strokeColor, m_style.m_strokeWidth));
	m_painter->setBrush(Qt::NoBrush);
	QPointF p1 = convertPoint(s.start());
	QPointF p2 = convertPoint(s.end());
	m_painter->drawLine(p1, p2);

	if (m_style.m_mode & vertices) {
		draw(s.start());
		draw(s.end());
	}
}

void GeometryWidget::draw(const Polygon& p) {
	setupPainter();
	QPainterPath path;
	addPolygonToPath(path, p);
	m_painter->drawPath(path);
}

void GeometryWidget::draw(const Polygon_with_holes& p) {
	setupPainter();
	QPainterPath path;
	addPolygonToPath(path, p.outer_boundary());
	for (auto hole : p.holes()) {
		addPolygonToPath(path, hole);
	}
	m_painter->drawPath(path);
}

void GeometryWidget::addPolygonToPath(QPainterPath& path, const Polygon& p) {
	for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
		if (vertex == p.vertices_begin()) {
			path.moveTo(convertPoint(*vertex));
		} else {
			path.lineTo(convertPoint(*vertex));
		}
	}
	path.closeSubpath();
}

void GeometryWidget::draw(const Circle& c) {
	setupPainter();
	QRectF rect = convertBox(c.bbox());
	m_painter->drawEllipse(rect);
}

void GeometryWidget::draw(const Box& b) {
	setupPainter();
	QRectF rect = convertBox(b);
	m_painter->drawRect(rect);
}

void GeometryWidget::drawText(const Point& p, const std::string& text) {
	setupPainter();
	QPointF p2 = convertPoint(p);
	m_painter->drawText(QRectF(p2 - QPointF{500, 250}, p2 + QPointF{500, 250}), Qt::AlignCenter,
	                    QString::fromStdString(text));
}

void GeometryWidget::setupPainter() {
	if (m_style.m_mode & GeometryRenderer::fill) {
		m_painter->setBrush(QBrush(m_style.m_fillColor));
	} else {
		m_painter->setBrush(Qt::NoBrush);
	}
	if (m_style.m_mode & GeometryRenderer::stroke) {
		m_painter->setPen(QPen(m_style.m_strokeColor, m_style.m_strokeWidth));
	} else {
		m_painter->setPen(Qt::NoPen);
	}
}

void GeometryWidget::pushStyle() {
	m_styleStack.push(m_style);
}

void GeometryWidget::popStyle() {
	m_style = m_styleStack.top();
	m_styleStack.pop();
}

void GeometryWidget::setMode(int mode) {
	m_style.m_mode = mode;
}

void GeometryWidget::setStroke(Color color, double width) {
	m_style.m_strokeColor = QColor(color.r, color.g, color.b);
	m_style.m_strokeWidth = width;
}

void GeometryWidget::setFill(Color color) {
	m_style.m_fillColor.setRgb(color.r, color.g, color.b, m_style.m_fillColor.alpha());
}

void GeometryWidget::setFillOpacity(int alpha) {
	m_style.m_fillColor.setAlpha(alpha);
}

/*std::unique_ptr<QPainter> GeometryWidget::getQPainter() {
	return std::make_unique<QPainter>(this);
}*/

void GeometryWidget::setDrawAxes(bool drawAxes) {
	m_drawAxes = drawAxes;
	update();
}

void GeometryWidget::setMinZoom(double minZoom) {
	m_minZoom = minZoom;
}

void GeometryWidget::setMaxZoom(double maxZoom) {
	m_maxZoom = maxZoom;
}

void GeometryWidget::zoomIn() {
	m_transform *= 1.5;
	if (m_transform.m11() > m_maxZoom) {
		m_transform *= m_maxZoom / m_transform.m11();
	}
	updateZoomSlider();
	update();
}

void GeometryWidget::zoomOut() {
	m_transform /= 1.5;
	if (m_transform.m11() < m_minZoom) {
		m_transform /= m_minZoom / m_transform.m11();
	}
	updateZoomSlider();
	update();
}

} // namespace renderer
} // namespace cartocrow