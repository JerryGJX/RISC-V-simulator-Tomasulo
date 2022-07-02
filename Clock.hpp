//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__CLOCK_HPP_
#define RISC_V_TOMASULO__CLOCK_HPP_

class Clock {
  int32_t present_time = 0;
 public:
  Clock() = default;

  void clockProceed() {//to show a step forward
    present_time++;
  }

  [[nodiscard]] int32_t getPresentTime() const {//to judge if the function can run
    return present_time;
  };

  [[nodiscard]] bool ifHighLevel() const {//if true, then can out
    return present_time % 2 == 0;
  }

  [[nodiscard]] bool ifLowLevel() const {//if true, then can't out
    return !ifHighLevel();
  }

};

#endif //RISC_V_TOMASULO__CLOCK_HPP_
