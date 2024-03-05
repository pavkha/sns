#include "observer.h"

namespace patterns {

void subject::attach(observer* o) {
    if (o != nullptr) {
        observers.push_back(o);
    }
}

void subject::dettach(observer* o) {
    if (o != nullptr) {
        observers.remove(o);
    }
}

void subject::notify() {
    for (auto o : observers) {
        o->update(this);
    }
}

}    // namespace patterns
