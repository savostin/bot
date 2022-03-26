#include "events.h"

Events *Events::_loop = NULL;

Events::Events(void) : q(), m(), c() {}

Events::~Events() {}

void Events::push(Event e) {
  std::lock_guard<std::mutex> lock(m);
  q.push(e);
  c.notify_one();
}

Event Events::pop(void) {
  std::unique_lock<std::mutex> lock(m);
  while (q.empty()) {
    c.wait(lock);
  }
  Event val = q.front();
  q.pop();
  return val;
}

Events *Events::loop() {
  if (_loop == NULL) {
    _loop = new Events();
  }
  return _loop;
}