#pragma once

namespace app {

class task {
  public:
    virtual ~task() = default;

    virtual bool init() = 0;
    virtual void run() = 0;
    virtual void deinit() = 0;
};

}    // namespace app
