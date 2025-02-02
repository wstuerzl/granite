/**
MIT License

This file is part of the Granite project which is based on Basalt.
https://github.com/DLR-RM/granite

Copyright (c) Martin Wudenka, Deutsches Zentrum für Luft- und Raumfahrt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
Original license of Basalt:
BSD 3-Clause License

This file is part of the Basalt project.
https://gitlab.com/VladyslavUsenko/granite-headers.git

Copyright (c) 2019, Vladyslav Usenko and Nikolaus Demmel.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@file
@brief Uniform B-spline for SE(3)
*/

#pragma once

#include <granite/spline/rd_spline.h>
#include <granite/spline/so3_spline.h>
#include <granite/utils/assert.h>

#include <granite/calibration/calib_bias.hpp>

#include <array>

namespace granite {

template <typename Scalar = double>
class Se3SplineBase {
 public:
  explicit Se3SplineBase(int64_t time_interval_ns) : dt_ns(time_interval_ns) {}
  virtual ~Se3SplineBase() = default;

  using Vec3 = Eigen::Matrix<Scalar, 3, 1>;
  using Vec6 = Eigen::Matrix<Scalar, 6, 1>;
  // using Vec9 = Eigen::Matrix<Scalar, 9, 1>;
  // using Vec12 = Eigen::Matrix<Scalar, 12, 1>;
  //
  // using Mat3 = Eigen::Matrix<Scalar, 3, 3>;
  // using Mat6 = Eigen::Matrix<Scalar, 6, 6>;
  //
  // using Mat36 = Eigen::Matrix<Scalar, 3, 6>;
  // using Mat39 = Eigen::Matrix<Scalar, 3, 9>;
  // using Mat312 = Eigen::Matrix<Scalar, 3, 12>;

  using SO3 = Sophus::SO3<Scalar>;
  using SE3 = Sophus::SE3<Scalar>;

  [[nodiscard]] virtual inline int getOrder() = 0;

  [[nodiscard]] virtual inline int getDegree() = 0;

  /// @brief Gererate random trajectory
  ///
  /// @param[in] n number of knots to generate
  /// @param[in] static_init if true the first N knots will be the same
  /// resulting in static initial condition
  virtual void genRandomTrajectory(int n, bool static_init = false) = 0;

  /// @brief Set the knot to particular SE(3) pose
  ///
  /// @param[in] pose SE(3) pose
  /// @param[in] i index of the knot
  virtual void setKnot(const Sophus::SE3d &pose, int i) = 0;

  /// @brief Reset spline to have num_knots initialized at pose
  ///
  /// @param[in] pose SE(3) pose
  /// @param[in] num_knots number of knots to initialize
  virtual void setKnots(const Sophus::SE3d &pose, int num_knots) = 0;

  /// @brief Add knot to the end of the spline
  ///
  /// @param[in] knot knot to add
  virtual inline void knots_push_back(const SE3 &knot) = 0;

  /// @brief Remove knot from the back of the spline
  virtual inline void knots_pop_back() = 0;

  /// @brief Return the first knot of the spline
  ///
  /// @return first knot of the spline
  virtual inline SE3 knots_front() const = 0;

  /// @brief Remove first knot of the spline and increase the start time
  virtual inline void knots_pop_front() = 0;

  /// @brief Return the last knot of the spline
  ///
  /// @return last knot of the spline
  virtual SE3 getLastKnot() = 0;

  /// @brief Return knot with index i
  ///
  /// @param i index of the knot
  /// @return knot
  SE3 getKnot(size_t i) const {
    SE3 res(getKnotSO3(i), getKnotPos(i));
    return res;
  }

  /// @brief Return reference to the SO(3) knot with index i
  ///
  /// @param i index of the knot
  /// @return reference to the SO(3) knot
  virtual inline SO3 &getKnotSO3(size_t i) = 0;

  /// @brief Return const reference to the SO(3) knot with index i
  ///
  /// @param i index of the knot
  /// @return const reference to the SO(3) knot
  virtual inline const SO3 &getKnotSO3(size_t i) const = 0;

  /// @brief Return reference to the position knot with index i
  ///
  /// @param i index of the knot
  /// @return reference to the position knot
  virtual inline Vec3 &getKnotPos(size_t i) = 0;

  /// @brief Return const reference to the position knot with index i
  ///
  /// @param i index of the knot
  /// @return const reference to the position knot
  virtual inline const Vec3 &getKnotPos(size_t i) const = 0;

  /// @brief Set start time for spline
  ///
  /// @param[in] start_time_ns start time of the spline in nanoseconds
  virtual inline void setStartTimeNs(int64_t s) = 0;

  /// @brief Maximum time represented by spline
  ///
  /// @return maximum time represented by spline in nanoseconds
  [[nodiscard]] virtual int64_t maxTimeNs() const = 0;

  /// @brief Minimum time represented by spline
  ///
  /// @return minimum time represented by spline in nanoseconds
  [[nodiscard]] virtual int64_t minTimeNs() const = 0;

  /// @brief Number of knots in the spline
  [[nodiscard]] virtual size_t numKnots() const = 0;

  /// @brief Linear acceleration in the world frame.
  ///
  /// @param[in] time_ns time to evaluate linear acceleration in nanoseconds
  virtual inline Vec3 transAccelWorld(int64_t time_ns) const = 0;

  /// @brief Linear velocity in the world frame.
  ///
  /// @param[in] time_ns time to evaluate linear velocity in nanoseconds
  virtual inline Vec3 transVelWorld(int64_t time_ns) const = 0;

  /// @brief Rotational velocity in the body frame.
  ///
  /// @param[in] time_ns time to evaluate rotational velocity in nanoseconds
  virtual inline Vec3 rotVelBody(int64_t time_ns) const = 0;

  /// @brief Evaluate pose.
  ///
  /// @param[in] time_ns time to evaluate pose in nanoseconds
  /// @return SE(3) pose at time_ns
  virtual SE3 pose(int64_t time_ns) const = 0;

  /// @brief Evaluate pose and compute time Jacobian.
  ///
  /// @param[in] time_ns time to evaluate pose in nanoseconds
  /// @param[out] J Jacobian of the pose with time
  virtual void d_pose_d_t(int64_t time_ns, Vec6 &J) const = 0;

  /// @brief Evaluate gyroscope residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement gyroscope measurement
  /// @param[in] gyro_bias_full gyroscope calibration
  /// @return gyroscope residual
  virtual Vec3 gyroResidual(
      int64_t time_ns, const Vec3 &measurement,
      const CalibGyroBias<Scalar> &gyro_bias_full) const = 0;

  /// @brief Evaluate accelerometer residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement accelerometer measurement
  /// @param[in] accel_bias_full accelerometer calibration
  /// @param[in] g gravity
  /// @return accelerometer residual
  virtual Vec3 accelResidual(int64_t time_ns,
                             const Eigen::Vector3d &measurement,
                             const CalibAccelBias<Scalar> &accel_bias_full,
                             const Eigen::Vector3d &g) const = 0;

  /// @brief Print knots for debugging.
  virtual inline void print_knots() const = 0;

  /// @brief Print position knots for debugging.
  virtual inline void print_pos_knots() const = 0;

  /// @brief Knot time interval in nanoseconds.
  [[nodiscard]] inline int64_t getDtNs() const { return dt_ns; }

  /// @brief Set time interval in nanoseconds
  virtual inline void setDtNs(int64_t new_dt_ns) { dt_ns = new_dt_ns; }

 protected:

  int64_t dt_ns;  ///< Knot interval in nanoseconds
};

/// @brief Uniform B-spline for SE(3) of order N. Internally uses an SO(3) (\ref
/// So3Spline) spline for rotation and 3D Euclidean spline (\ref RdSpline) for
/// translation (split representaion).
///
/// See [[arXiv:1911.08860]](https://arxiv.org/abs/1911.08860) for more details.
template <int _N, typename Scalar = double>
class Se3Spline : public Se3SplineBase<Scalar> {
 public:
  using Base = Se3SplineBase<Scalar>;

  static constexpr int N = _N;        ///< Order of the spline.
  static constexpr int DEG = _N - 1;  ///< Degree of the spline.

  using MatN = Eigen::Matrix<Scalar, _N, _N>;
  using VecN = Eigen::Matrix<Scalar, _N, 1>;
  using VecNp1 = Eigen::Matrix<Scalar, _N + 1, 1>;

  using Vec3 = Eigen::Matrix<Scalar, 3, 1>;
  using Vec6 = Eigen::Matrix<Scalar, 6, 1>;
  using Vec9 = Eigen::Matrix<Scalar, 9, 1>;
  using Vec12 = Eigen::Matrix<Scalar, 12, 1>;

  using Mat3 = Eigen::Matrix<Scalar, 3, 3>;
  using Mat6 = Eigen::Matrix<Scalar, 6, 6>;

  using Mat36 = Eigen::Matrix<Scalar, 3, 6>;
  using Mat39 = Eigen::Matrix<Scalar, 3, 9>;
  using Mat312 = Eigen::Matrix<Scalar, 3, 12>;

  using Matrix3Array = std::array<Mat3, N>;
  using Matrix36Array = std::array<Mat36, N>;
  using Matrix6Array = std::array<Mat6, N>;

  using SO3 = Sophus::SO3<Scalar>;
  using SE3 = Sophus::SE3<Scalar>;

  using PosJacobianStruct = typename RdSpline<3, N, Scalar>::JacobianStruct;
  using SO3JacobianStruct = typename So3Spline<N, Scalar>::JacobianStruct;

  /// @brief Struct to store the accelerometer residual Jacobian with
  /// respect to knots
  struct AccelPosSO3JacobianStruct {
    size_t start_idx;
    std::array<Mat36, N> d_val_d_knot;
  };

  /// @brief Struct to store the pose Jacobian with respect to knots
  struct PosePosSO3JacobianStruct {
    size_t start_idx;
    std::array<Mat6, N> d_val_d_knot;
  };

  /// @brief Constructor with knot interval and start time
  ///
  /// @param[in] time_interval_ns knot time interval in nanoseconds
  /// @param[in] start_time_ns start time of the spline in nanoseconds
  explicit Se3Spline(int64_t time_interval_ns, int64_t start_time_ns = 0)
      : Base(time_interval_ns),
        pos_spline(time_interval_ns, start_time_ns),
        so3_spline(time_interval_ns, start_time_ns) {}

  [[nodiscard]] inline int getOrder() override { return N; }

  [[nodiscard]] inline int getDegree() override { return DEG; }

  /// @brief Gererate random trajectory
  ///
  /// @param[in] n number of knots to generate
  /// @param[in] static_init if true the first N knots will be the same
  /// resulting in static initial condition
  void genRandomTrajectory(int n, bool static_init = false) override {
    so3_spline.genRandomTrajectory(n, static_init);
    pos_spline.genRandomTrajectory(n, static_init);
  }

  /// @brief Set the knot to particular SE(3) pose
  ///
  /// @param[in] pose SE(3) pose
  /// @param[in] i index of the knot
  void setKnot(const Sophus::SE3d &pose, int i) override {
    so3_spline.getKnot(i) = pose.so3();
    pos_spline.getKnot(i) = pose.translation();
  }

  /// @brief Reset spline to have num_knots initialized at pose
  ///
  /// @param[in] pose SE(3) pose
  /// @param[in] num_knots number of knots to initialize
  void setKnots(const Sophus::SE3d &pose, int num_knots) override {
    so3_spline.resize(num_knots);
    pos_spline.resize(num_knots);

    for (int i = 0; i < num_knots; i++) {
      so3_spline.getKnot(i) = pose.so3();
      pos_spline.getKnot(i) = pose.translation();
    }
  }

  /// @brief Reset spline to the knots from other spline
  ///
  /// @param[in] other spline to copy knots from
  void setKnots(const Se3Spline<N, Scalar> &other) {
    GRANITE_ASSERT(other.dt_ns == Base::dt_ns);
    GRANITE_ASSERT(other.pos_spline.getKnots().size() ==
                  other.pos_spline.getKnots().size());

    size_t num_knots = other.pos_spline.getKnots().size();

    so3_spline.resize(num_knots);
    pos_spline.resize(num_knots);

    for (size_t i = 0; i < num_knots; i++) {
      so3_spline.getKnot(i) = other.so3_spline.getKnot(i);
      pos_spline.getKnot(i) = other.pos_spline.getKnot(i);
    }
  }

  /// @brief Add knot to the end of the spline
  ///
  /// @param[in] knot knot to add
  inline void knots_push_back(const SE3 &knot) override {
    so3_spline.knots_push_back(knot.so3());
    pos_spline.knots_push_back(knot.translation());
  }

  /// @brief Remove knot from the back of the spline
  inline void knots_pop_back() override {
    so3_spline.knots_pop_back();
    pos_spline.knots_pop_back();
  }

  /// @brief Return the first knot of the spline
  ///
  /// @return first knot of the spline
  inline SE3 knots_front() const override {
    SE3 res(so3_spline.knots_front(), pos_spline.knots_front());

    return res;
  }

  /// @brief Remove first knot of the spline and increase the start time
  inline void knots_pop_front() override {
    so3_spline.knots_pop_front();
    pos_spline.knots_pop_front();

    GRANITE_ASSERT(so3_spline.minTimeNs() == pos_spline.minTimeNs());
    GRANITE_ASSERT(so3_spline.getKnots().size() == pos_spline.getKnots().size());
  }

  /// @brief Return the last knot of the spline
  ///
  /// @return last knot of the spline
  SE3 getLastKnot() override {
    GRANITE_ASSERT(so3_spline.getKnots().size() == pos_spline.getKnots().size());

    SE3 res(so3_spline.getKnots().back(), pos_spline.getKnots().back());

    return res;
  }

  /// @brief Return reference to the SO(3) knot with index i
  ///
  /// @param i index of the knot
  /// @return reference to the SO(3) knot
  inline SO3 &getKnotSO3(size_t i) override { return so3_spline.getKnot(i); }

  /// @brief Return const reference to the SO(3) knot with index i
  ///
  /// @param i index of the knot
  /// @return const reference to the SO(3) knot
  inline const SO3 &getKnotSO3(size_t i) const override {
    return so3_spline.getKnot(i);
  }

  /// @brief Return reference to the position knot with index i
  ///
  /// @param i index of the knot
  /// @return reference to the position knot
  inline Vec3 &getKnotPos(size_t i) override { return pos_spline.getKnot(i); }

  /// @brief Return const reference to the position knot with index i
  ///
  /// @param i index of the knot
  /// @return const reference to the position knot
  inline const Vec3 &getKnotPos(size_t i) const override {
    return pos_spline.getKnot(i);
  }

  /// @brief Set start time for spline
  ///
  /// @param[in] start_time_ns start time of the spline in nanoseconds
  inline void setStartTimeNs(int64_t s) override {
    so3_spline.setStartTimeNs(s);
    pos_spline.setStartTimeNs(s);
  }

  /// @brief Apply increment to the knot
  ///
  /// The increment vector consists of translational and rotational parts \f$
  /// [\upsilon, \omega]^T \f$. Given the current pose of the knot \f$ R \in
  /// SO(3), p \in \mathbb{R}^3\f$ the updated pose is: \f{align}{ R' &=
  /// \exp(\omega) R
  /// \\ p' &= p + \upsilon
  /// \f}
  ///  The increment is consistent with \ref
  /// PoseState::applyInc.
  ///
  /// @param[in] i index of the knot
  /// @param[in] inc 6x1 increment vector
  template <typename Derived>
  void applyInc(int i, const Eigen::MatrixBase<Derived> &inc) {
    EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 6);

    pos_spline.getKnot(i) += inc.template head<3>();
    so3_spline.getKnot(i) =
        SO3::exp(inc.template tail<3>()) * so3_spline.getKnot(i);
  }

  /// @brief Maximum time represented by spline
  ///
  /// @return maximum time represented by spline in nanoseconds
  [[nodiscard]] int64_t maxTimeNs() const override {
    GRANITE_ASSERT_STREAM(so3_spline.maxTimeNs() == pos_spline.maxTimeNs(),
                         "so3_spline.maxTimeNs() " << so3_spline.maxTimeNs()
                                                   << " pos_spline.maxTimeNs() "
                                                   << pos_spline.maxTimeNs());
    return pos_spline.maxTimeNs();
  }

  /// @brief Minimum time represented by spline
  ///
  /// @return minimum time represented by spline in nanoseconds
  [[nodiscard]] int64_t minTimeNs() const override {
    GRANITE_ASSERT_STREAM(so3_spline.minTimeNs() == pos_spline.minTimeNs(),
                         "so3_spline.minTimeNs() " << so3_spline.minTimeNs()
                                                   << " pos_spline.minTimeNs() "
                                                   << pos_spline.minTimeNs());
    return pos_spline.minTimeNs();
  }

  /// @brief Number of knots in the spline
  [[nodiscard]] size_t numKnots() const override {
    return pos_spline.getKnots().size();
  }

  /// @brief Linear acceleration in the world frame.
  ///
  /// @param[in] time_ns time to evaluate linear acceleration in nanoseconds
  inline Vec3 transAccelWorld(int64_t time_ns) const override {
    return pos_spline.acceleration(time_ns);
  }

  /// @brief Linear velocity in the world frame.
  ///
  /// @param[in] time_ns time to evaluate linear velocity in nanoseconds
  inline Vec3 transVelWorld(int64_t time_ns) const override {
    return pos_spline.velocity(time_ns);
  }

  /// @brief Rotational velocity in the body frame.
  ///
  /// @param[in] time_ns time to evaluate rotational velocity in nanoseconds
  inline Vec3 rotVelBody(int64_t time_ns) const override {
    return so3_spline.velocityBody(time_ns);
  }

  /// @brief Evaluate pose.
  ///
  /// @param[in] time_ns time to evaluate pose in nanoseconds
  /// @return SE(3) pose at time_ns
  SE3 pose(int64_t time_ns) const override {
    SE3 res;

    res.so3() = so3_spline.evaluate(time_ns);
    res.translation() = pos_spline.evaluate(time_ns);

    return res;
  }

  /// @brief Evaluate pose and compute Jacobian.
  ///
  /// @param[in] time_ns time to evaluate pose in nanoseconds
  /// @param[out] J Jacobian of the pose with respect to knots
  /// @return SE(3) pose at time_ns
  Sophus::SE3d pose(int64_t time_ns, PosePosSO3JacobianStruct *J) const {
    Sophus::SE3d res;

    typename So3Spline<_N, Scalar>::JacobianStruct Jr;
    typename RdSpline<3, N, Scalar>::JacobianStruct Jp;

    res.so3() = so3_spline.evaluate(time_ns, &Jr);
    res.translation() = pos_spline.evaluate(time_ns, &Jp);

    if (J) {
      Eigen::Matrix3d RT = res.so3().inverse().matrix();

      J->start_idx = Jr.start_idx;
      for (int i = 0; i < N; i++) {
        J->d_val_d_knot[i].setZero();
        J->d_val_d_knot[i].template topLeftCorner<3, 3>() =
            RT * Jp.d_val_d_knot[i];
        J->d_val_d_knot[i].template bottomRightCorner<3, 3>() =
            RT * Jr.d_val_d_knot[i];
      }
    }

    return res;
  }

  /// @brief Evaluate pose and compute time Jacobian.
  ///
  /// @param[in] time_ns time to evaluate pose in nanoseconds
  /// @param[out] J Jacobian of the pose with time
  void d_pose_d_t(int64_t time_ns, Vec6 &J) const override {
    J.template head<3>() =
        so3_spline.evaluate(time_ns).inverse() * transVelWorld(time_ns);
    J.template tail<3>() = rotVelBody(time_ns);
  }

  /// @brief Evaluate gyroscope residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement gyroscope measurement
  /// @param[in] gyro_bias_full gyroscope calibration
  /// @return gyroscope residual
  Vec3 gyroResidual(
      int64_t time_ns, const Vec3 &measurement,
      const CalibGyroBias<Scalar> &gyro_bias_full) const override {
    return so3_spline.velocityBody(time_ns) -
           gyro_bias_full.getCalibrated(measurement);
  }

  /// @brief Evaluate gyroscope residual and compute Jacobians.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement gyroscope measurement
  /// @param[in] gyro_bias_full gyroscope calibration
  /// @param[out] J_knots Jacobian with respect to SO(3) spline knots
  /// @param[out] J_bias Jacobian with respect to gyroscope calibration
  /// @return gyroscope residual
  Vec3 gyroResidual(int64_t time_ns, const Vec3 &measurement,
                    const CalibGyroBias<Scalar> &gyro_bias_full,
                    SO3JacobianStruct *J_knots,
                    Mat312 *J_bias = nullptr) const {
    if (J_bias) {
      J_bias->setZero();
      J_bias->template block<3, 3>(0, 0).diagonal().array() = 1.0;
      J_bias->template block<3, 3>(0, 3).diagonal().array() = -measurement[0];
      J_bias->template block<3, 3>(0, 6).diagonal().array() = -measurement[1];
      J_bias->template block<3, 3>(0, 9).diagonal().array() = -measurement[2];
    }

    return so3_spline.velocityBody(time_ns, J_knots) -
           gyro_bias_full.getCalibrated(measurement);
  }

  /// @brief Evaluate accelerometer residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement accelerometer measurement
  /// @param[in] accel_bias_full accelerometer calibration
  /// @param[in] g gravity
  /// @return accelerometer residual
  Vec3 accelResidual(int64_t time_ns, const Eigen::Vector3d &measurement,
                     const CalibAccelBias<Scalar> &accel_bias_full,
                     const Eigen::Vector3d &g) const override {
    Sophus::SO3d R = so3_spline.evaluate(time_ns);
    Eigen::Vector3d accel_world = pos_spline.acceleration(time_ns);

    return R.inverse() * (accel_world + g) -
           accel_bias_full.getCalibrated(measurement);
  }

  /// @brief Evaluate accelerometer residual and Jacobians.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measurement accelerometer measurement
  /// @param[in] accel_bias_full accelerometer calibration
  /// @param[in] g gravity
  /// @param[out] J_knots Jacobian with respect to spline knots
  /// @param[out] J_bias Jacobian with respect to accelerometer calibration
  /// @param[out] J_g Jacobian with respect to gravity
  /// @return accelerometer residual
  Vec3 accelResidual(int64_t time_ns, const Vec3 &measurement,
                     const CalibAccelBias<Scalar> &accel_bias_full,
                     const Vec3 &g, AccelPosSO3JacobianStruct *J_knots,
                     Mat39 *J_bias = nullptr, Mat3 *J_g = nullptr) const {
    typename So3Spline<_N, Scalar>::JacobianStruct Jr;
    typename RdSpline<3, N, Scalar>::JacobianStruct Jp;

    Sophus::SO3d R = so3_spline.evaluate(time_ns, &Jr);
    Eigen::Vector3d accel_world = pos_spline.acceleration(time_ns, &Jp);

    Eigen::Matrix3d RT = R.inverse().matrix();
    Eigen::Matrix3d tmp = RT * Sophus::SO3d::hat(accel_world + g);

    GRANITE_ASSERT_STREAM(
        Jr.start_idx == Jp.start_idx,
        "Jr.start_idx " << Jr.start_idx << " Jp.start_idx " << Jp.start_idx);

    GRANITE_ASSERT_STREAM(
        so3_spline.getKnots().size() == pos_spline.getKnots().size(),
        "so3_spline.getKnots().size() " << so3_spline.getKnots().size()
                                        << " pos_spline.getKnots().size() "
                                        << pos_spline.getKnots().size());

    J_knots->start_idx = Jp.start_idx;
    for (int i = 0; i < N; i++) {
      J_knots->d_val_d_knot[i].template topLeftCorner<3, 3>() =
          RT * Jp.d_val_d_knot[i];
      J_knots->d_val_d_knot[i].template bottomRightCorner<3, 3>() =
          tmp * Jr.d_val_d_knot[i];
    }

    if (J_bias) {
      J_bias->setZero();
      J_bias->template block<3, 3>(0, 0).diagonal().array() = 1.0;
      J_bias->template block<3, 3>(0, 3).diagonal().array() = -measurement[0];
      (*J_bias)(1, 6) = -measurement[1];
      (*J_bias)(2, 7) = -measurement[1];
      (*J_bias)(2, 8) = -measurement[2];
    }
    if (J_g) (*J_g) = RT;

    Vec3 res =
        RT * (accel_world + g) - accel_bias_full.getCalibrated(measurement);

    return res;
  }

  /// @brief Evaluate position residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measured_position position measurement
  /// @param[out] Jp if not nullptr, Jacobian with respect to knos of the
  /// position spline
  /// @return position residual
  Sophus::Vector3d positionResidual(int64_t time_ns,
                                    const Vec3 &measured_position,
                                    PosJacobianStruct *Jp = nullptr) const {
    return pos_spline.evaluate(time_ns, Jp) - measured_position;
  }

  /// @brief Evaluate orientation residual.
  ///
  /// @param[in] time_ns time of the measurement
  /// @param[in] measured_orientation orientation measurement
  /// @param[out] Jr if not nullptr, Jacobian with respect to knos of the
  /// SO(3) spline
  /// @return orientation residual
  Sophus::Vector3d orientationResidual(int64_t time_ns,
                                       const SO3 &measured_orientation,
                                       SO3JacobianStruct *Jr = nullptr) const {
    Sophus::Vector3d res =
        (so3_spline.evaluate(time_ns, Jr) * measured_orientation.inverse())
            .log();

    if (Jr) {
      Eigen::Matrix3d Jrot;
      Sophus::leftJacobianSO3(res, Jrot);

      for (int i = 0; i < N; i++) {
        Jr->d_val_d_knot[i] = Jrot * Jr->d_val_d_knot[i];
      }
    }

    return res;
  }

  /// @brief Print knots for debugging.
  inline void print_knots() const override {
    for (size_t i = 0; i < pos_spline.getKnots().size(); i++) {
      std::cout << i << ": p:" << pos_spline.getKnot(i).transpose() << " q: "
                << so3_spline.getKnot(i).unit_quaternion().coeffs().transpose()
                << std::endl;
    }
  }

  /// @brief Print position knots for debugging.
  inline void print_pos_knots() const override {
    for (size_t i = 0; i < pos_spline.getKnots().size(); i++) {
      std::cout << pos_spline.getKnot(i).transpose() << std::endl;
    }
  }

  /// @brief Set time interval in nanoseconds
  inline void setDtNs(int64_t new_dt_ns) override {
    Base::setDtNs(new_dt_ns);
    pos_spline.setTimeIntervalNs(new_dt_ns);
    so3_spline.setTimeIntervalNs(new_dt_ns);
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

 private:
  RdSpline<3, _N, Scalar> pos_spline;  ///< Position spline
  So3Spline<_N, Scalar> so3_spline;    ///< Orientation spline
};

}  // namespace granite
