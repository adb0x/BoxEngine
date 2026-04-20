#ifndef CONTACTLISTENER_H
#define CONTACTLISTENER_H

#include "box2d/box2d.h"

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

#endif