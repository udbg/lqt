#include "lqt_qt.hpp"

#include <QVariant>

#include <QBitArray>
#include <QByteArray>
#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QEasingCurve>
#include <QHash>
#include <QLine>
#include <QLineF>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QRect>
#include <QRectF>
#include <QRegExp>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QUrl>

#if defined(MODULE_qtgui) || defined(MODULE_qtwidgets)
#include <QKeySequence>
#include <QBitmap>
#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QImage>
#include <QMatrix>
#include <QMatrix4x4>
#include <QPalette>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QPolygon>
#include <QQuaternion>
#include <QRegion>

#ifdef MODULE_qtwidgets
#include <QSizePolicy>
#endif

#include <QTextFormat>
#include <QTextLength>
#include <QTransform>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

// require qtgui variant types
#ifndef MODULE_qtgui
#define MODULE_qtgui
#endif
#endif

static int lqtL_qvariant_setValue(lua_State *L) {
	QVariant* self = static_cast<QVariant*>(lqtL_toudata(L, 1, "QVariant*"));
	lqtL_selfcheck(L, self, "QVariant");
	/* basic types */
	if (lua_isnil(L, 2)) {
		self->clear();
	} else if (lua_isboolean(L, 2)) {
		self->setValue((bool)lua_toboolean(L, 2));
	} else if (lua_isnumber(L, 2)) {
		self->setValue((double)lua_tonumber(L, 2));
	} else if (lua_isstring(L, 2)) {
		size_t size;
		const char * str = lua_tolstring(L, 2, &size);
		self->setValue(QByteArray(str, size));
	} /* QtCore types */
	else if (lqtL_testudata(L, 2, "QBitArray*")) {
		self->setValue(*(QBitArray*)lqtL_toudata(L, 2, "QBitArray*"));
	} else if (lqtL_testudata(L, 2, "QByteArray*")) {
		self->setValue(*(QByteArray*)lqtL_toudata(L, 2, "QByteArray*"));
	} else if (lqtL_testudata(L, 2, "QChar*")) {
		self->setValue(*(QChar*)lqtL_toudata(L, 2, "QChar*"));
	} else if (lqtL_testudata(L, 2, "QDate*")) {
		self->setValue(*(QDate*)lqtL_toudata(L, 2, "QDate*"));
	} else if (lqtL_testudata(L, 2, "QDateTime*")) {
		self->setValue(*(QDateTime*)lqtL_toudata(L, 2, "QDateTime*"));
	} else if (lqtL_testudata(L, 2, "QLine*")) {
		self->setValue(*(QLine*)lqtL_toudata(L, 2, "QLine*"));
	} else if (lqtL_testudata(L, 2, "QLineF*")) {
		self->setValue(*(QLineF*)lqtL_toudata(L, 2, "QLineF*"));
	} else if (lqtL_testudata(L, 2, "QList<QVariant>*")) {
		self->setValue(*(QList<QVariant>*)lqtL_toudata(L, 2, "QList<QVariant>*"));
	} else if (lqtL_testudata(L, 2, "QLocale*")) {
		self->setValue(*(QLocale*)lqtL_toudata(L, 2, "QLocale*"));
	} else if (lqtL_testudata(L, 2, "QRect*")) {
		self->setValue(*(QRect*)lqtL_toudata(L, 2, "QRect*"));
	} else if (lqtL_testudata(L, 2, "QRectF*")) {
		self->setValue(*(QRectF*)lqtL_toudata(L, 2, "QRectF*"));
	} else if (lqtL_testudata(L, 2, "QRegExp*")) {
		self->setValue(*(QRegExp*)lqtL_toudata(L, 2, "QRegExp*"));
	} else if (lqtL_testudata(L, 2, "QSize*")) {
		self->setValue(*(QSize*)lqtL_toudata(L, 2, "QSize*"));
	} else if (lqtL_testudata(L, 2, "QSizeF*")) {
		self->setValue(*(QSizeF*)lqtL_toudata(L, 2, "QSizeF*"));
	} else if (lqtL_testudata(L, 2, "QString*")) {
		self->setValue(*(QString*)lqtL_toudata(L, 2, "QString*"));
	} else if (lqtL_testudata(L, 2, "QStringList*")) {
		self->setValue(*(QStringList*)lqtL_toudata(L, 2, "QStringList*"));
	} else if (lqtL_testudata(L, 2, "QTime*")) {
		self->setValue(*(QTime*)lqtL_toudata(L, 2, "QTime*"));
	} else if (lqtL_testudata(L, 2, "QUrl*")) {
		self->setValue(*(QUrl*)lqtL_toudata(L, 2, "QUrl*"));

#ifdef MODULE_qtgui	
	/* QtGui types */
	} else if (lqtL_testudata(L, 2, "QKeySequence*")) {
		self->setValue(*(QKeySequence*)lqtL_toudata(L, 2, "QKeySequence*"));
	} else if (lqtL_testudata(L, 2, "QBitmap*")) {
		self->setValue(*(QBitmap*)lqtL_toudata(L, 2, "QBitmap*"));
	} else if (lqtL_testudata(L, 2, "QBrush*")) {
		self->setValue(*(QBrush*)lqtL_toudata(L, 2, "QBrush*"));
	} else if (lqtL_testudata(L, 2, "QColor*")) {
		self->setValue(*(QColor*)lqtL_toudata(L, 2, "QColor*"));
	} else if (lqtL_testudata(L, 2, "QCursor*")) {
		self->setValue(*(QCursor*)lqtL_toudata(L, 2, "QCursor*"));
	} else if (lqtL_testudata(L, 2, "QFont*")) {
		self->setValue(*(QFont*)lqtL_toudata(L, 2, "QFont*"));
	} else if (lqtL_testudata(L, 2, "QIcon*")) {
		self->setValue(*(QIcon*)lqtL_toudata(L, 2, "QIcon*"));
	} else if (lqtL_testudata(L, 2, "QImage*")) {
		self->setValue(*(QImage*)lqtL_toudata(L, 2, "QImage*"));
	} else if (lqtL_testudata(L, 2, "QMatrix*")) {
		self->setValue(*(QMatrix*)lqtL_toudata(L, 2, "QMatrix*"));
	} else if (lqtL_testudata(L, 2, "QMatrix4x4*")) {
		self->setValue(*(QMatrix4x4*)lqtL_toudata(L, 2, "QMatrix4x4*"));
	} else if (lqtL_testudata(L, 2, "QPalette*")) {
		self->setValue(*(QPalette*)lqtL_toudata(L, 2, "QPalette*"));
	} else if (lqtL_testudata(L, 2, "QPen*")) {
		self->setValue(*(QPen*)lqtL_toudata(L, 2, "QPen*"));
	} else if (lqtL_testudata(L, 2, "QPoint*")) {
		self->setValue(*(QPoint*)lqtL_toudata(L, 2, "QPoint*"));
	} else if (lqtL_testudata(L, 2, "QPointF*")) {
		self->setValue(*(QPointF*)lqtL_toudata(L, 2, "QPointF*"));
	} else if (lqtL_testudata(L, 2, "QPixmap*")) {
		self->setValue(*(QPixmap*)lqtL_toudata(L, 2, "QPixmap*"));
	} else if (lqtL_testudata(L, 2, "QPolygon*")) {
		self->setValue(*(QPolygon*)lqtL_toudata(L, 2, "QPolygon*"));
	} else if (lqtL_testudata(L, 2, "QQuaternion*")) {
		self->setValue(*(QQuaternion*)lqtL_toudata(L, 2, "QQuaternion*"));
	} else if (lqtL_testudata(L, 2, "QRegion*")) {
		self->setValue(*(QRegion*)lqtL_toudata(L, 2, "QRegion*"));
	} else if (lqtL_testudata(L, 2, "QTextFormat*")) {
		self->setValue(*(QTextFormat*)lqtL_toudata(L, 2, "QTextFormat*"));
	} else if (lqtL_testudata(L, 2, "QTextLength*")) {
		self->setValue(*(QTextLength*)lqtL_toudata(L, 2, "QTextLength*"));
	} else if (lqtL_testudata(L, 2, "QTransform*")) {
		self->setValue(*(QTransform*)lqtL_toudata(L, 2, "QTransform*"));
	} else if (lqtL_testudata(L, 2, "QVector2D*")) {
		self->setValue(*(QVector2D*)lqtL_toudata(L, 2, "QVector2D*"));
	} else if (lqtL_testudata(L, 2, "QVector3D*")) {
		self->setValue(*(QVector3D*)lqtL_toudata(L, 2, "QVector3D*"));
	} else if (lqtL_testudata(L, 2, "QVector4D*")) {
		self->setValue(*(QVector4D*)lqtL_toudata(L, 2, "QVector4D*"));
#endif

#ifdef MODULE_qtwidgets
	} else if (lqtL_testudata(L, 2, "QSizePolicy*")) {
		self->setValue(*(QSizePolicy*)lqtL_toudata(L, 2, "QSizePolicy*"));
#endif

	} else if (lqtL_testudata(L, 2, "QObject*")) {
		QObject* obj = static_cast<QObject*>(lqtL_toudata(L, 2, "QObject*"));
		self->setValue(obj);
	}
	return 0;
}

int lqtL_qvariant_value_custom(lua_State *L, int index, bool convert_to) {
	QVariant* self = static_cast<QVariant*>(lqtL_toudata(L, index, "QVariant*"));
	lqtL_selfcheck(L, self, "QVariant");
	QVariant::Type type;
	if (!convert_to || lua_isnoneornil(L, 2)) {
		type = self->type();
	} else {
		type = (QVariant::Type)lqtL_toenum(L, 2, "QVariant.Type");
		const char * currentType = self->typeName();
		if (!self->canConvert(type) || !self->convert(type)) {
			lua_pushnil(L);
			lua_pushfstring(L, "cannot convert %s to %s", currentType, self->typeToName(type));
			return 2;
		}
	}

	if(self->type() == (int) QMetaType::QObjectStar) {

		lqtL_pushqobject(L, self->value<QObject*>());
		return 1;
	}

	switch (self->type()) {
		case QVariant::Invalid: lua_pushnil(L); return 1;
		/* basic types */
		case QVariant::Bool: lua_pushboolean(L, self->toBool()); return 1;
		case QVariant::Double:
			lua_pushnumber(L, self->toDouble()); return 1;
		case QVariant::Int:
		case QVariant::LongLong:
			lua_pushinteger(L, self->toLongLong()); return 1;
		case QVariant::UInt:
		case QVariant::ULongLong:
			lua_pushinteger(L, self->toULongLong()); return 1;
		case QVariant::ByteArray: {
			const QByteArray &ba = self->toByteArray();
			lua_pushlstring(L, ba.data(), ba.size());
			return 1;
		};
		/* QtCore types */
		case QVariant::BitArray: lqtL_passudata(L, new QBitArray(self->value<QBitArray>()), "QBitArray*"); return 1;
		case QVariant::Char: lqtL_passudata(L, new QChar(self->value<QChar>()), "QChar*"); return 1;
		case QVariant::Date: lqtL_passudata(L, new QDate(self->value<QDate>()), "QDate*"); return 1;
		case QVariant::DateTime: lqtL_passudata(L, new QDateTime(self->value<QDateTime>()), "QDateTime*"); return 1;
		case QVariant::Line: lqtL_passudata(L, new QLine(self->value<QLine>()), "QLine*"); return 1;
		case QVariant::LineF: lqtL_passudata(L, new QLineF(self->value<QLineF>()), "QLineF*"); return 1;
		case QVariant::List: lqtL_passudata(L, new QList<QVariant>(self->toList()), "QList<QVariant>*"); return 1;
		case QVariant::Locale: lqtL_passudata(L, new QLocale(self->value<QLocale>()), "QLocale*"); return 1;
		case QVariant::Point: lqtL_passudata(L, new QPoint(self->value<QPoint>()), "QPoint*"); return 1;
		case QVariant::PointF: lqtL_passudata(L, new QPointF(self->value<QPointF>()), "QPointF*"); return 1;
		case QVariant::Rect: lqtL_passudata(L, new QRect(self->value<QRect>()), "QRect*"); return 1;
		case QVariant::RectF: lqtL_passudata(L, new QRectF(self->value<QRectF>()), "QRectF*"); return 1;
		case QVariant::RegExp: lqtL_passudata(L, new QRegExp(self->value<QRegExp>()), "QRegExp*"); return 1;
		case QVariant::Size: lqtL_passudata(L, new QSize(self->value<QSize>()), "QSize*"); return 1;
		case QVariant::SizeF: lqtL_passudata(L, new QSizeF(self->value<QSizeF>()), "QSizeF*"); return 1;
		case QVariant::String: lqtL_passudata(L, new QString(self->value<QString>()), "QString*"); return 1;
		case QVariant::StringList: lqtL_passudata(L, new QStringList(self->value<QStringList>()), "QStringList*"); return 1;
		case QVariant::Time: lqtL_passudata(L, new QTime(self->value<QTime>()), "QTime*"); return 1;
        case QVariant::Url: lqtL_passudata(L, new QUrl(self->value<QUrl>()), "QUrl*"); return 1;
        case QVariant::Map: lqtL_passudata(L, new QVariantMap(self->value<QVariantMap>()), "QVariantMap*"); return 1;
        case QVariant::Hash: lqtL_passudata(L, new QVariantHash(self->value<QVariantHash>()), "QVariantHash*"); return 1;
        case QVariant::EasingCurve: lqtL_passudata(L, new QEasingCurve(self->value<QEasingCurve>()), "QEasingCurve*"); return 1;
        case QVariant::Uuid: lqtL_passudata(L, new QUuid(self->value<QUuid>()), "QUuid*"); return 1;
        case QVariant::ModelIndex: lqtL_passudata(L, new QModelIndex(self->value<QModelIndex>()), "QModelIndex*"); return 1;
        case QVariant::PersistentModelIndex: lqtL_passudata(L, new QPersistentModelIndex(self->value<QPersistentModelIndex>()), "QPersistentModelIndex*"); return 1;
        case QVariant::RegularExpression: lqtL_passudata(L, new QRegularExpression(self->toRegularExpression()), "QRegularExpression*"); return 1;
        case QVariant::LastCoreType: lqtL_passudata(L, new QCborMap(self->value<QCborMap>()), "QCborMap*"); return 1;

#ifdef MODULE_qtgui
		/* QtGui types */
        case QVariant::PolygonF: lqtL_passudata(L, new QPolygonF(self->value<QPolygonF>()), "QPolygonF*"); return 1;
		case QVariant::KeySequence: lqtL_passudata(L, new QKeySequence(self->value<QKeySequence>()), "QKeySequence*"); return 1;
		case QVariant::Bitmap: lqtL_passudata(L, new QBitmap(self->value<QBitmap>()), "QBitmap*"); return 1;
		case QVariant::Brush: lqtL_passudata(L, new QBrush(self->value<QBrush>()), "QBrush*"); return 1;
		case QVariant::Color: lqtL_passudata(L, new QColor(self->value<QColor>()), "QColor*"); return 1;
		case QVariant::Cursor: lqtL_passudata(L, new QCursor(self->value<QCursor>()), "QCursor*"); return 1;
		case QVariant::Font: lqtL_passudata(L, new QFont(self->value<QFont>()), "QFont*"); return 1;
		case QVariant::Icon: lqtL_passudata(L, new QIcon(self->value<QIcon>()), "QIcon*"); return 1;
		case QVariant::Image: lqtL_passudata(L, new QImage(self->value<QImage>()), "QImage*"); return 1;
		case QVariant::Matrix: lqtL_passudata(L, new QMatrix(self->value<QMatrix>()), "QMatrix*"); return 1;
		case QVariant::Matrix4x4: lqtL_passudata(L, new QMatrix4x4(self->value<QMatrix4x4>()), "QMatrix4x4*"); return 1;
		case QVariant::Palette: lqtL_passudata(L, new QPalette(self->value<QPalette>()), "QPalette*"); return 1;
		case QVariant::Pen: lqtL_passudata(L, new QPen(self->value<QPen>()), "QPen*"); return 1;
		case QVariant::Pixmap: lqtL_passudata(L, new QPixmap(self->value<QPixmap>()), "QPixmap*"); return 1;
		case QVariant::Polygon: lqtL_passudata(L, new QPolygon(self->value<QPolygon>()), "QPolygon*"); return 1;
		case QVariant::Quaternion: lqtL_passudata(L, new QQuaternion(self->value<QQuaternion>()), "QQuaternion*"); return 1;
		case QVariant::Region: lqtL_passudata(L, new QRegion(self->value<QRegion>()), "QRegion*"); return 1;
		case QVariant::Transform: lqtL_passudata(L, new QTransform(self->value<QTransform>()), "QTransform*"); return 1;
		case QVariant::TextFormat: lqtL_passudata(L, new QTextFormat(self->value<QTextFormat>()), "QTextFormat*"); return 1;
		case QVariant::TextLength: lqtL_passudata(L, new QTextLength(self->value<QTextLength>()), "QTextLength*"); return 1;
		case QVariant::Vector2D: lqtL_passudata(L, new QVector2D(self->value<QVector2D>()), "QVector2D*"); return 1;
		case QVariant::Vector3D: lqtL_passudata(L, new QVector3D(self->value<QVector3D>()), "QVector3D*"); return 1;
		case QVariant::Vector4D: lqtL_passudata(L, new QVector4D(self->value<QVector4D>()), "QVector4D*"); return 1;
#else
		// [avoid] warning: xxx enumeration values not handled in switch: [xxx] [xxx] [xxx] ...
        case QVariant::PolygonF:
		case QVariant::KeySequence:
		case QVariant::Bitmap:
		case QVariant::Brush:
		case QVariant::Color:
		case QVariant::Cursor:
		case QVariant::Font:
		case QVariant::Icon:
		case QVariant::Image:
		case QVariant::Matrix:
		case QVariant::Matrix4x4:
		case QVariant::Palette:
		case QVariant::Pen:
		case QVariant::Pixmap:
		case QVariant::Polygon:
		case QVariant::Quaternion:
		case QVariant::Region:
		case QVariant::Transform:
		case QVariant::TextFormat:
		case QVariant::TextLength:
		case QVariant::Vector2D:
		case QVariant::Vector3D:
		case QVariant::Vector4D:
			break;
#endif

#ifdef MODULE_qtwidgets
		case QVariant::SizePolicy: lqtL_passudata(L, new QSizePolicy(self->value<QSizePolicy>()), "QSizePolicy*"); return 1;
#else
		case QVariant::SizePolicy:
			break;
#endif

        case QVariant::UserType:
        case QVariant::LastType:
        	break;
	}
	return 0;
}

static int lqtL_qvariant_value(lua_State *L) {
    return lqtL_qvariant_value_custom(L, 1, true);
}

void lqtL_qvariant_custom(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, "QVariant*");
    if (!lua_istable(L, -1))
        luaL_error(L, "QVariant not registered!");
	int qvariant = lua_gettop(L);

	lua_pushliteral(L, "value");
	lua_pushcfunction(L, lqtL_qvariant_value);
	lua_rawset(L, qvariant);

	lua_pushliteral(L, "setValue");
	lua_pushcfunction(L, lqtL_qvariant_setValue);
	lua_rawset(L, qvariant);
}

void lqtL_qvariant_custom_qtgui(lua_State *L)
{
	return lqtL_qvariant_custom(L);
}