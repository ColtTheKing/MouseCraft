#pragma once

#include "../Util/TypePunners.h"
#include "../Input/InputSystem.h"
#include "NetState.h"
#include <algorithm>
#include <string>
#include "NetworkComponent.h"

constexpr size_t MAX_DATUM_SIZE = 256;	// temporary fix to send larger amounts of data 

class NetDatum {
public:
    enum DataType {
        ACK = 0x00,
        CONNECTION_REQUEST = 0x01,
        CONNECTION_ACCEPT = 0x02,
        HOST_INFO_REQUEST = 0x03,
        HOST_INFO_RESPONSE = 0x04,
        TRANSFORM_STATE_UPDATE = 0x10,
        ENTITY_CREATE = 0x11,
        ENTITY_DESTROY = 0x12,
        EVENT_TRIGGER = 0x20,
        PLAYER_AXIS = 0x30,
        PLAYER_BUTTON = 0x31,
    };

    const unsigned char GetType() const { return IntChar{_type}.c[0]; }
    const unsigned char * GetPointer() const { return _data; };
    const size_t GetSize() const { return _size; };

    virtual const bool IsReliable() const = 0;
protected:
    NetDatum(const DataType type) : _type(type), _size(0) {};

    void appendByte(const unsigned char byte) {
        if (MAX_DATUM_SIZE - _size > sizeof(byte)) {
            _data[_size++] = byte;
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendBool(const bool b) {
        if (MAX_DATUM_SIZE - _size > sizeof(b)) {
            BoolChar data = { b };
            std::copy(data.c, data.c + sizeof(b), _data + _size);
            _size += sizeof(b);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendShort(const short s) {
        if (MAX_DATUM_SIZE - _size > sizeof(s)) {
            ShortChar data = { s };
            std::copy(data.c, data.c + sizeof(s), _data + _size);
            _size += sizeof(s);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendUShort(const unsigned short s) {
        if (MAX_DATUM_SIZE - _size > sizeof(s)) {
            UShortChar data = { s };
            std::copy(data.c, data.c + sizeof(s), _data + _size);
            _size += sizeof(s);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendFloat(const float f) {
        if (MAX_DATUM_SIZE - _size > sizeof(f)) {
            FloatChar data = { f };
            std::copy(data.c, data.c + sizeof(f), _data + _size);
            _size += sizeof(f);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendInt(const int i) {
        if (MAX_DATUM_SIZE - _size > sizeof(i)) {
            IntChar data = { i };
            std::copy(data.c, data.c + sizeof(i), _data + _size);
            _size += sizeof(i);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendUInt(const unsigned int i) {
        if (MAX_DATUM_SIZE - _size > sizeof(i)) {
            UIntChar data = { i };
            std::copy(data.c, data.c + sizeof(i), _data + _size);
            _size += sizeof(i);
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    void appendString(const std::string str) {
        if (MAX_DATUM_SIZE - _size > str.size() + sizeof(unsigned int)) {
            appendUInt(str.size());
            const char * cstr = str.c_str();
            std::copy(cstr, cstr + str.size(), _data + _size);
            _size += str.size();
		} else {
			SDL_assert(false && "ERROR: Datum is too large!");
		}
    }

    DataType _type;

    size_t _size;
    unsigned char _data[MAX_DATUM_SIZE];
};

class AckDatum : public NetDatum {
public:
    AckDatum(const unsigned short tickNum) : NetDatum(NetDatum::ACK) {
        appendUShort(tickNum);
    }

    const bool IsReliable() const override { return false; }
};

class ConnReqDatum : public NetDatum {
public:
    ConnReqDatum() : NetDatum(NetDatum::CONNECTION_REQUEST) { }

    const bool IsReliable() const override { return true; }
};

class ConnAccDatum : public NetDatum {
public:
    ConnAccDatum(const unsigned short tickNum) : NetDatum(NetDatum::CONNECTION_ACCEPT) { 
        appendUShort(tickNum);
    }

    const bool IsReliable() const override { return true; }
};

class InfoReqDatum : public NetDatum {
public:
    InfoReqDatum() : NetDatum(NetDatum::HOST_INFO_REQUEST) { }

    const bool IsReliable() const override { return false; }
};

class InfoResDatum : public NetDatum {
public:
    InfoResDatum(const unsigned short numPlayers) : NetDatum(NetDatum::HOST_INFO_RESPONSE) {
        appendShort(numPlayers);
    }

    const bool IsReliable() const override { return false; }
};

class StateUpdateDatum : public NetDatum {
public:
    StateUpdateDatum(unsigned int id, const NetState & state) : NetDatum(NetDatum::TRANSFORM_STATE_UPDATE) {
        appendUInt(id);

		appendUInt(state.parentID);
        appendBool(state.enabled);

        appendFloat(state.pos.x);
        appendFloat(state.pos.y);
        appendFloat(state.pos.z);

        appendFloat(state.rot.x);
        appendFloat(state.rot.y);
        appendFloat(state.rot.z);

        appendFloat(state.scl.x);
        appendFloat(state.scl.y);
        appendFloat(state.scl.z);
    }

    const bool IsReliable() const override { return false; }
};

class EntityCreateDatum : public NetDatum {
public:
    EntityCreateDatum(const NetworkComponent *component) : NetDatum(NetDatum::ENTITY_CREATE) {
        appendUInt(component->GetNetworkID());

        Entity *entity = component->GetEntity();
        
        unsigned int parentID = 0;
        Entity *parent = entity->GetParent();
        if (parent != nullptr) {
            NetworkComponent * comp = parent->GetComponent<NetworkComponent>();
            if (comp != nullptr)
                parentID = comp->GetNetworkID();
        }

        appendUInt(parentID);
        appendBool(entity->GetEnabled());

        appendFloat(entity->transform.getLocalPosition().x);
        appendFloat(entity->transform.getLocalPosition().y);
        appendFloat(entity->transform.getLocalPosition().z);

        appendFloat(entity->transform.getLocalRotation().x);
        appendFloat(entity->transform.getLocalRotation().y);
        appendFloat(entity->transform.getLocalRotation().z);

        appendFloat(entity->transform.getLocalScale().x);
        appendFloat(entity->transform.getLocalScale().y);
        appendFloat(entity->transform.getLocalScale().z);

        appendString(component->GetComponentData());
    }

    const bool IsReliable() const override { return true; }
};

class EntityDestroyDatum : public NetDatum {
public:
    EntityDestroyDatum(const NetworkComponent *component) : NetDatum(NetDatum::ENTITY_DESTROY) {
        appendUInt(component->GetNetworkID());
    }

    const bool IsReliable() const override { return true; }
};

class PlayerAxisDatum : public NetDatum {
public:
    PlayerAxisDatum(Axis2DEvent *eventData) : NetDatum(NetDatum::PLAYER_AXIS) {
        appendInt(eventData->axis);
        appendFloat(eventData->value.x);
        appendFloat(eventData->value.y);
    }

    const bool IsReliable() const override { return false; }
};

class PlayerButtonDatum : public NetDatum {
public:
    PlayerButtonDatum(ButtonEvent *eventData) : NetDatum(NetDatum::PLAYER_BUTTON) {
		appendInt(eventData->button);
        appendBool(eventData->isDown);
    }

    const bool IsReliable() const override { return false; }
};