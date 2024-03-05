#pragma once

#include <list>

namespace patterns {

class observer;

class subject {
  public:
    subject(const subject&) = delete;
    subject& operator=(const subject&) = delete;
    virtual ~subject() = default;

    void attach(observer*);
    void dettach(observer*);
    void notify();

  protected:
    subject() = default;

  private:
    std::list<observer*> observers{};
};

class observer {
  public:
    virtual ~observer() = default;

    virtual void update(subject*) = 0;
};

}    // namespace patterns
