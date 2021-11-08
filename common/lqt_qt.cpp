#include "lqt_qt.hpp"

// #include <QThread>

#include <QMetaObject>
#include <QMetaMethod>

// #define VERBOSE_BUILD

#define CASE(x) case QMetaMethod::x : lua_pushstring(L, " " #x); break
static int lqtL_methods(lua_State *L) {
	QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
	if (self == NULL)
		return luaL_argerror(L, 1, "expecting QObject*");
	const QMetaObject *mo = self->metaObject();
	lua_createtable(L, mo->methodCount(), 0);
	for (int i=0; i < mo->methodCount(); i++) {
		QMetaMethod m = mo->method(i);
		lua_pushstring(L, m.methodSignature());
		switch (m.access()) {
		CASE(Private);
		CASE(Protected);
		CASE(Public);
		}
		switch (m.methodType()) {
		CASE(Method);
		CASE(Signal);
		CASE(Slot);
		CASE(Constructor);
		}
		lua_concat(L, 3);
		lua_rawseti(L, -2, m.methodIndex());
	}
	return 1;
}
#undef CASE

int lqtL_pushqobject(lua_State *L, QObject * object) {
    if (object == nullptr) {
        lua_pushnil(L);
        return 0;
    }
    const QMetaObject * meta = object->metaObject();
    while (meta) {
        QString className = meta->className();
        className += "*";
        char * cname = strdup(qPrintable(className));
        lua_getfield(L, LUA_REGISTRYINDEX, cname);
        int isnil = lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!isnil) {
            lqtL_pushudata(L, object, cname);
            free(cname);
            return 1;
        } else {
            free(cname);
            meta = meta->superClass();
        }
    }
    QString className = meta->className();
    luaL_error(L, "QObject `%s` not registered!", className.toStdString().c_str());
    return 0;
}

static int lqtL_findchild(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

    QString name = luaL_checkstring(L, 2);
    QObject * child = self->findChild<QObject*>(name);

    if (child) {
        lqtL_pushqobject(L, child);
        return 1;
    } else {
        return 0;
    }
}

static int lqtL_children(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");
    const QObjectList & children = self->children();

    lua_newtable(L);
    for (int i=0; i < children.count(); i++) {
        QObject * object = children[i];
        QString name = object->objectName();
        if (!name.isEmpty() && lqtL_pushqobject(L, object)) {
            lua_setfield(L, -2, qPrintable(name));
        }
    }
    return 1;
}

static int lqtL_findChildren(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

    auto option = Qt::FindChildOptions(lua_tointeger(L, 3));
    QObjectList children;
    if (auto re = static_cast<QRegularExpression*>(lqtL_toudata(L, 2, "QRegularExpression*")))
    {
        children = self->findChildren<QObject*>(*re, option);
    }
    else
    {
        children = self->findChildren<QObject*>(luaL_optstring(L, 2, ""), option);
    }

    // return as table
    if (lua_toboolean(L, 4))
    {
        lua_newtable(L);
        for (int i = 0; i < children.count(); i++) {
            QObject * object = children[i];
            QString name = object->objectName();
            if (!name.isEmpty() && lqtL_pushqobject(L, object)) {
                lua_setfield(L, -2, qPrintable(name));
            }
        }
    }
    else
    {
        lua_createtable(L, children.count(), 0);
        for (int i = 0; i < children.count(); i++) {
            QObject * object = children[i];
            if (lqtL_pushqobject(L, object)) {
                lua_rawseti(L, -2, i + 1);
            }
        }
    }
    return 1;
}

static int lqtL_connect(lua_State *L) {
    static int methodId = 0;

    QObject* sender = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (sender == NULL)
        return luaL_argerror(L, 1, "sender not QObject*");

    const char *signal = luaL_checkstring(L, 2);
    const QMetaObject *senderMeta = sender->metaObject();
    int idxS = senderMeta->indexOfSignal(signal + 1);
    if (idxS == -1)
        return luaL_argerror(L, 2, qPrintable(QString("no such sender signal: '%1'").arg(signal + 1)));

    QObject* receiver;
    QString methodName;

    if (lua_type(L, 3) == LUA_TFUNCTION) {
        receiver = sender;

        // simulate sender:__addmethod('LQT_SLOT_X(signature)', function()...end)
        QMetaMethod m = senderMeta->method(idxS);
        methodName = QString(m.methodSignature()).replace(QRegExp("^[^\\(]+"), QString("LQT_SLOT_%1").arg(methodId++));

        lua_getfield(L, 1, "__addslot");
        lua_pushvalue(L, 1);
        lua_pushstring(L, qPrintable(methodName));
        lua_pushvalue(L, 3);

        if(lqtL_pcall(L, 3, 0, 0) != 0)
            lua_error(L);

        methodName.prepend("1");

#ifdef VERBOSE_BUILD
        printf("Connect method (%p) %d(`%s`) to lua-method `%s`\n"
            , receiver
            , idxS
            , signal
            , methodName.toStdString().c_str()
        );
#endif
    } else {
        receiver = static_cast<QObject*>(lqtL_toudata(L, 3, "QObject*"));
        if (receiver == NULL)
            return luaL_argerror(L, 3, "receiver not QObject*");
        const char *method = luaL_checkstring(L, 4);
        methodName = method;

        const QMetaObject *receiverMeta = receiver->metaObject();
        int idxR = receiverMeta->indexOfMethod(method + 1);
        if (idxR == -1)
            return luaL_argerror(L, 4, qPrintable(QString("no such receiver method: '%1'").arg(method + 1)));

#ifdef VERBOSE_BUILD
        printf("Connect method (%p) %d(`%s`) to method (%p) %d(`%s`)\n"
            , sender
            , idxS
            , signal
            , receiver
            , idxR
            , method
        );
#endif
    }

    bool ok = QObject::connect(sender, signal, receiver, qPrintable(methodName));
    lua_pushboolean(L, ok);
    return 1;
}

static int lqtL_metaObject(lua_State *L) {

    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

	lqtL_pushudata(L, self->metaObject(), "QMetaObject*");
	return 1;
}

void lqtL_getrefclasstable(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, LQT_REF_CLASS);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_REF_CLASS);
    }
}

int lqtL_setErrorHandler(lua_State *L) {

    if(!lua_isfunction(L, 1))
        luaL_typerror(L, 1, "function");

    lqtL_getrefclasstable(L);
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "errorHandler");
    lua_pop(L, 1);

    return 0;
}

static int lqtL___ptr(lua_State *L) {
    if (auto self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*")))
    {
        lua_pushinteger(L, lua_Integer(self));
        return 1;
    }
    return 0;
}

static int lqtL_cast(lua_State *L) {
    if (auto self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*")))
    {
        return lqtL_pushqobject(L, self);
    }
    return 0;
}

static int lqt_metaMethod_invoke(lua_State *L) {
    int oldtop = lua_gettop(L);
    QMetaMethod* self = static_cast<QMetaMethod*>(lqtL_toudata(L, 1, "QMetaMethod*"));
    lqtL_selfcheck(L, self, "QMetaMethod");

    size_t data[10] = { 0 };
    QGenericReturnArgument retarg(self->typeName(), data);
    switch (self->returnType())
    {
#define INIT_ARG_CASE(T) case QMetaType::T: \
        { \
            T val; \
            memcpy(data, &val, sizeof(val)); \
            break; \
        } \

    INIT_ARG_CASE(QByteArray)
    INIT_ARG_CASE(QString)
    INIT_ARG_CASE(QStringList)
    INIT_ARG_CASE(QVariantList)
    INIT_ARG_CASE(QVariantMap)
    };

    QObject* arg1 = static_cast<QObject*>(lqtL_toudata(L, 2, "QObject*"));
    Qt::ConnectionType arg2 = static_cast<Qt::ConnectionType>(lqtL_toenum(L, 3, "Qt.ConnectionType"));
    QGenericArgument arg3 = lua_isnoneornil(L, 4) ? static_cast< QGenericArgument >(QGenericArgument(NULL)) : lqtL_getGenericArgument(L, 4, self->parameterType(0));
    QGenericArgument arg4 = lua_isnoneornil(L, 5) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 5, self->parameterType(1));
    QGenericArgument arg5 = lua_isnoneornil(L, 6) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 6, self->parameterType(2));
    QGenericArgument arg6 = lua_isnoneornil(L, 7) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 7, self->parameterType(3));
    QGenericArgument arg7 = lua_isnoneornil(L, 8) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 8, self->parameterType(4));
    QGenericArgument arg8 = lua_isnoneornil(L, 9) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 9, self->parameterType(5));
    QGenericArgument arg9 = lua_isnoneornil(L, 10) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 10, self->parameterType(6));
    QGenericArgument arg10 = lua_isnoneornil(L, 11) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 11, self->parameterType(7));
    QGenericArgument arg11 = lua_isnoneornil(L, 12) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 12, self->parameterType(8));
    QGenericArgument arg12 = lua_isnoneornil(L, 13) ? static_cast< QGenericArgument >(QGenericArgument()) : lqtL_getGenericArgument(L, 13, self->parameterType(9));
    bool ret = self->invoke(arg1, arg2, retarg, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
    if (!ret) return 0;

    switch (self->returnType())
    {
    case QMetaType::Int:
        lua_pushinteger(L, lua_Integer(*(int*)data));
        break;
    case QMetaType::UInt:
        lua_pushinteger(L, lua_Integer(*(uint*)data));
        break;
    case QMetaType::Long:
        lua_pushinteger(L, lua_Integer(*(long*)data));
        break;
    case QMetaType::ULong:
        lua_pushinteger(L, lua_Integer(*(ulong*)data));
        break;
    case QMetaType::LongLong:
        lua_pushinteger(L, lua_Integer(*(qlonglong*)data));
        break;
    case QMetaType::ULongLong:
        lua_pushinteger(L, lua_Integer(*(qulonglong*)data));
        break;
    case QMetaType::Bool:
        lua_pushboolean(L, *(bool*)data);
        break;
    case QMetaType::Float:
        lua_pushnumber(L, lua_Number(*(float*)data));
        break;
    case QMetaType::Double:
        lua_pushnumber(L, lua_Number(*(double*)data));
        break;
    case QMetaType::Short:
        lua_pushinteger(L, lua_Integer(*(short*)data));
        break;
    case QMetaType::UShort:
        lua_pushinteger(L, lua_Integer(*(ushort*)data));
        break;
    case QMetaType::Char:
        lua_pushinteger(L, lua_Integer(*(char*)data));
        break;
    case QMetaType::UChar:
        lua_pushinteger(L, lua_Integer(*(uchar*)data));
        break;

#define ARG_CASE(T) case QMetaType::T: \
        lqtL_pushudata(L, new T(std::move(*(T*)data)), #T"*"); \
        break;

    ARG_CASE(QString)
    ARG_CASE(QStringList)
    ARG_CASE(QByteArray)

    ARG_CASE(QVariant)
    ARG_CASE(QVariantList)
    ARG_CASE(QVariantMap)

    ARG_CASE(QDate)
    ARG_CASE(QTime)
    ARG_CASE(QDateTime)

    ARG_CASE(QSize)
    ARG_CASE(QRect)
    ARG_CASE(QLine)
    ARG_CASE(QPoint)
    ARG_CASE(QSizeF)
    ARG_CASE(QRectF)
    ARG_CASE(QLineF)
    ARG_CASE(QPointF)

    // ARG_CASE(QFont)
    // ARG_CASE(QPixmap)
    // ARG_CASE(QBrush)
    // ARG_CASE(QColor)
    // ARG_CASE(QPalette)
    // ARG_CASE(QIcon)
    // ARG_CASE(QImage)
    // ARG_CASE(QPolygon)
    // ARG_CASE(QRegion)
    // ARG_CASE(QBitmap)
    // ARG_CASE(QCursor)
    // ARG_CASE(QKeySequence)
    // ARG_CASE(QPen)
    // ARG_CASE(QTextLength)
    // ARG_CASE(QTextFormat)
    // ARG_CASE(QMatrix)
    // ARG_CASE(QTransform)
    // ARG_CASE(QMatrix4x4)
    // ARG_CASE(QVector2D)
    // ARG_CASE(QVector3D)
    // ARG_CASE(QVector4D)
    // ARG_CASE(QQuaternion)
    // ARG_CASE(QPolygonF)

    default:
        lua_pushnil(L);
    }
    return 1;
}

void lqtL_qobject_custom (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "QObject*");
    int qobject = lua_gettop(L);

    lqtL_embed(L);

    lua_pushstring(L, "__methods");
    lua_pushcfunction(L, lqtL_methods);
    lua_rawset(L, qobject);

    lua_pushstring(L, "__ptr");
    lua_pushcfunction(L, lqtL___ptr);
    lua_rawset(L, qobject);

    lua_pushstring(L, "cast");
    lua_pushcfunction(L, lqtL_cast);
    lua_rawset(L, qobject);

    lua_pushstring(L, "findChild");
    lua_pushcfunction(L, lqtL_findchild);
    lua_rawset(L, qobject);

    lua_pushstring(L, "children");
    lua_pushcfunction(L, lqtL_children);
    lua_rawset(L, qobject);

    lua_pushstring(L, "findChildren");
    lua_pushcfunction(L, lqtL_findChildren);
    lua_rawset(L, qobject);

    lua_pushstring(L, "connect");
    lua_pushcfunction(L, lqtL_connect);
    lua_rawset(L, qobject);

    lua_pushstring(L, "metaObject");
    lua_pushcfunction(L, lqtL_metaObject);
    lua_rawset(L, qobject);

    // also modify the static QObject::connect function
    lua_getfield(L, -2, "QObject");
    lua_pushcfunction(L, lqtL_connect);
    lua_setfield(L, -2, "connect");
    // set QtCore.setErrorHandler function
    lua_pushcfunction(L, lqtL_setErrorHandler);
    lua_setfield(L, -4, "setErrorHandler");

    lua_getfield(L, LUA_REGISTRYINDEX, "QMetaMethod*");
    int qmetamethod = lua_gettop(L);

    lua_pushstring(L, "invoke2");
    lua_pushcfunction(L, lqt_metaMethod_invoke);
    lua_rawset(L, qmetamethod);
}


QList<QByteArray> lqtL_getStringList(lua_State *L, int i) {
    QList<QByteArray> ret;
    int n = lua_objlen(L, i);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_gettable(L, i);
        ret[i] = QByteArray(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return ret;
}

void lqtL_pushStringList(lua_State *L, const QList<QByteArray> &table) {
    const int n = table.size();
    lua_createtable(L, n, 0);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_pushstring(L, table[i].data());
        lua_settable(L, -3);
    }
}
