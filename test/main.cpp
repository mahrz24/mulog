#include <mulog/core>
#include "other.h"

#include <iostream>

int main(int argc, char *argv[])
{
  std::cout << "Hello World" << std::endl;

  mulog::core::get().set_min_severity(mulog::verbose);
  mulog::core::get().add_transformer<mulog::default_transformer,
    mulog::console_device>(mulog::prefix::severity);
  mulog::core::get().add_transformer<mulog::default_transformer,
    mulog::file_device>(mulog::prefix::extended, "log.txt");

  //mulog::core::get().add_filter(mulog::filters::tag(R"(core.*)"));

  INFO << "hello world";
  LOG(error) << "some error";

  //mulog::core::get().add_filter(mulog::filters::file("main.cpp"));

  LOG_TAG(debug, "core.inner") << "test";
  LOG_TAG(debug, "coreinner") << "test1";
  LOG_TAG(debug, "plugin.inner") << "test2";

  BLOCK_BEGIN << "Testblock";

  HEADER << "Hello World";

  INFO << "Bla";

  BLOCK_END;


  test();

  return 0;
}
