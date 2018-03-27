#pragma once

#include "grpl/profile/profile.h"

namespace grpl {
namespace profile {

  // TODO: can I expand this to be n-dimensional?
  // that is, expand it so that ORDER is the derivative
  // that is capable of changing instantly (acceleration for trapezoidal)
  class trapezoidal1 : public profile1<3> {
  public:
    segment_t calculate(segment_t &last, double time) const override {
      double dt = time - last.time;
      double slice_count_d = static_cast<double>(dt / _timeslice);
      int slice_count = static_cast<int>(slice_count_d);
      if (slice_count_d - slice_count > 0.9) slice_count++;
      if (slice_count < 1) slice_count++;

      double vel_max = _limits[1];
      double accel_max = _limits[2];

      segment_t seg = last;

      for (int i = 1; i <= slice_count; i++) {
        double t = seg.time + _timeslice;
        dt = t - seg.time;

        double error = seg.vect[0] - _goal;
        double accel = (error < 0 ? accel_max : -accel_max);

        // TODO: Find point at which we reach v_max and if it's less than dt, split
        // this slice into half.
        double v_projected = seg.vect[1] + accel*dt;
        v_projected = v_projected > vel_max ? vel_max : v_projected < -vel_max ? -vel_max : v_projected;

        double decel_time = v_projected / accel;
        double decel_dist = v_projected * decel_time - 0.5*accel*decel_time*decel_time;
        double decel_error = seg.vect[0] + decel_dist - _goal;

        // TODO: make this better
        // If we decelerate now, do we cross the zero of the error function?
        bool decel_cross_error_zeros = (error > 0 && decel_error < 0) || (error < 0 && decel_error > 0);
        // Are we not currently decelerating?
        bool decel_not_in_progress = (error < 0 && seg.vect[1] > 0) || (error > 0 && seg.vect[1] < 0);
        if (decel_cross_error_zeros && decel_not_in_progress)
          accel = -accel;

        double vel = seg.vect[1] + (accel * dt);
        seg.vect[0] = seg.vect[0] + (seg.vect[1] * dt) + (0.5 * accel * dt * dt);
        seg.vect[1] = vel > vel_max ? vel_max : vel < -vel_max ? -vel_max : vel;
        seg.vect[2] = accel;
        seg.time = t;
      }
      return seg;
    }
  };

} // namespace profile
} // namespace grpl