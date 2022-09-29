#include <covent/exceptions.hh>

covent::broken_promise::broken_promise()
  : std::logic_error("broken promise") {
  /* empty */
}
