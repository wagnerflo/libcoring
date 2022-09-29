cmake_minimum_required( VERSION 3.15 )
project( libcovent_examples )

find_package ( covent REQUIRED PATHS .. )

add_executable(
  sleep
  sleep.cc
)

target_link_libraries( sleep covent )
