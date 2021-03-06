#include "kalman_filter.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;

Tools tools;

// Please note that the Eigen library does not initialize 
// VectorXd or MatrixXd objects with zeros upon creation.

KalmanFilter::KalmanFilter() {}

KalmanFilter::~KalmanFilter() {}

void KalmanFilter::Init(VectorXd &x_in, MatrixXd &P_in, MatrixXd &F_in,
                        MatrixXd &H_in, MatrixXd &R_in, MatrixXd &Q_in) {
  x_ = x_in;
  P_ = P_in;
  F_ = F_in;
  H_ = H_in;
  R_ = R_in;
  Q_ = Q_in;
}

void KalmanFilter::Predict() {
  /**
  TODO:
    * predict the state
  */
  x_ = F_ * x_;
  MatrixXd Ft = F_.transpose();
  P_ = F_ * P_ * Ft + Q_;
}

void KalmanFilter::Update(const VectorXd &z) {
  /**
  TODO:
    * update the state by using Extended Kalman Filter equations
  */
  VectorXd z_pred = H_ * x_;
  VectorXd y = z - z_pred;
  MatrixXd Ht = H_.transpose();
  MatrixXd S = H_ * P_ * Ht + R_;
  MatrixXd Si = S.inverse();
  MatrixXd PHt = P_ * Ht;
  MatrixXd K = PHt * Si;

  //new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * H_) * P_;
}

VectorXd hFunction(const VectorXd& x_state) {
  /**
  TODO:
    * Calculate the h function here.
  */
  VectorXd hfunction(3);
  //recover state parameters
  double px, py, vx, vy, ro, phi, ro_dot;
  px = x_state(0);
  py = x_state(1);
  vx = x_state(2);
  vy = x_state(3);
  ro = sqrt(pow(px, 2.0) + pow(py, 2.0));
  
  // Avoid division by zero
  if(ro < 0.001){
    ro = 0.001;
  }
  // For small values of px or py, phi and radials speed are negligible
  if(fabs(px) < 0.001 || fabs(ro) < 0.001){
    phi = 0;
    ro_dot = 0;
  }
  phi = atan2(py, px);
  ro_dot = (px*vx + py*vy)/ro;
  

  /*
  // Avoid division by zero
  if(fabs(px) < 0.0001 || fabs(ro) < 0.0001){
    cout << "hFunction () - Error - Division by Zero" << endl;
    return h;
    }
  */

  //compute the h function
  hfunction << ro, phi, ro_dot;

  return hfunction;
}

void KalmanFilter::UpdateEKF(const VectorXd &z) {
  /**
  TODO:
    * update the state by using Kalman Filter equations
  */
  
  // Calculate the h function, see tools.cpp for more info
  VectorXd z_pred = hFunction(x_);
  VectorXd y = z - z_pred;
  
  // Make sure that the phi component of y is within pi and -pi
  while ( y(1) > M_PI || y(1) < -M_PI ) {
    if ( y(1) > M_PI ) {
      y(1) -= 2.0*M_PI;
    } else {
      y(1) += 2.0*M_PI;
    }
  }
  MatrixXd Hj = tools.CalculateJacobian(x_);
  MatrixXd Hjt = Hj.transpose();
  MatrixXd S = Hj * P_ * Hjt + R_;
  MatrixXd Si = S.inverse();
  MatrixXd PHt = P_ * Hjt;
  MatrixXd K = PHt * Si;

  //new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * Hj) * P_;
}