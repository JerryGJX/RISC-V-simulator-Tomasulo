//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__EXCEPTIONAL_HPP_
#define RISC_V_TOMASULO__EXCEPTIONAL_HPP_

#include <ostream>
#include <utility>
class Error : public std::exception
{
  const std::string message;

 public:
  Error() = default;
  explicit Error(std::string error_disc) : message(std::move(error_disc)){};
  Error(const Error &x) : message(x.message){};
  friend std::ostream &operator<<(std::ostream &os, const Error &error)
  {
    os << "message: " << error.message;
    return os;
  }
};

#endif //RISC_V_TOMASULO__EXCEPTIONAL_HPP_
