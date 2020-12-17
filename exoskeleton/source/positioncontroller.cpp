
#include "positioncontroller.h"


PositionController::PositionController(double hipJointRefAngle, double kneeJointRefAngle,
                                       double ankleJointRefAngle, Model* aModel) :
    Controller(),k_exohip K_EXOHIP,
    k_exoknee K_EXOKNEE,
    k_exoankle K_EXOANKLE,
    hipJointRefAngle(hipJointRefAngle),
    kneeJointRefAngle(kneeJointRefAngle),
    ankleJointRefAngle(ankleJointRefAngle)
{

}


/**
 * @brief PositionController::getComponentsOfTwoVec3: compute the main components between two points
 * @param initialPoint
 * @param finalPoint
 * @param vectorResult
 */
void PositionController::getComponentsOfTwoVec3(SimTK::Vec3 initialPoint, SimTK::Vec3 finalPoint, SimTK::Vec3* vectorResult) const
{

    SimTK::Vec3 vectResult(finalPoint[0]-initialPoint[0],finalPoint[1]-initialPoint[1],finalPoint[2]-initialPoint[2]);
    *vectorResult = vectResult;

}

double PositionController::getVectorModule(SimTK::Vec3& vector) const
{
    double module = sqrt(pow(vector[0],2) + pow(vector[1],2) + pow(vector[2],2));
    return module;
}

double PositionController::computeDesiredAngularAccelerationHipJoint(const State &s,
                                                                     double* getErr = nullptr,
                                                                     double* getPos = nullptr) const
{    
    // Obtain the coordinateset of the joint desired
    const Coordinate& coord = _model->getCoordinateSet().get("exoHipJoint_coord");
    double xt = coord.getValue(s); // Actual angle x(t)
    double tsample = 0.033; // Time sample
    if(getPos != nullptr)
       *getPos = xt; // Save position
    static double errt1 = hipJointRefAngle;
    double err = hipJointRefAngle - xt; // Error
    double iErr = (err + errt1)*tsample; // Compute integration
    double dErr = (err - errt1)/tsample; // Compute the derivative of xt
    errt1 = err;
    if(getErr != nullptr)
        *getErr = err;
    // cout << "Error ExoHipJoint = " << err << endl;
    double  kp = k_exohip[0], kv = k_exohip[1], ki = k_exohip[2];
    double desAcc = kp*err + kv*dErr + ki*iErr;
    return desAcc;
}

double PositionController::computeDesiredAngularAccelerationKneeJoint(const State &s,
                                                                      double* getErr = nullptr,
                                                                      double* getPos = nullptr) const
{
    const Coordinate& coord = _model->getCoordinateSet().get("kneeJoint_r_coord");
    double xt = coord.getValue(s);
    double tsample = 0.033;
    if(getPos != nullptr)
       *getPos = xt;
    static double errt1 = kneeJointRefAngle;
    double err = kneeJointRefAngle - xt; // Error
    double iErr = (err + errt1)*tsample; // Compute integration
    double dErr = (err - errt1)/tsample; // Compute the derivative of xt
    errt1 = err;
    if(getErr != nullptr)
        *getErr = err;
    //cout << "Error ExoKneeJoint = " << err << endl;
    double  kp = k_exoknee[0], kv = k_exoknee[1], ki = k_exoknee[2];
    double desAcc = kp*err + kv*dErr + ki*iErr;
    return desAcc;
}

double PositionController::computeDesiredAngularAccelerationAnkleJoint(const State &s,
                                                                       double* getErr = nullptr,
                                                                       double* getPos = nullptr) const
{
     const Coordinate& coord = _model->getCoordinateSet().get("ankleJoint_r_coord");
     double xt = coord.getValue(s);
     double tsample = 0.033;
     if(getPos != nullptr)
        *getPos = xt;
     static double xt1 = 0; // Angle position one previous step x(t-1)
     static double errt1 = ankleJointRefAngle;
     double err = ankleJointRefAngle - xt; // Error
     double iErr = (err + errt1)*tsample; // Compute integration
     double dErr = (err - errt1)/tsample; // Compute the derivative of xt
     errt1 = err;
     if(getErr != nullptr)
         *getErr = err;
     // cout << "Error ExoAnkleJoint = " << err << endl;
     double  kp = k_exoankle[0], kv = k_exoankle[1], ki = k_exoankle[2];
     double desAcc = kp*err + kv*dErr + ki*iErr;
     return desAcc;
}

SimTK::Vector PositionController::computeDynamicsTorque(const State &s,
                                                        double* getErrs = nullptr,
                                                        double* getPoses = nullptr) const
{
    double desAccExoHip = computeDesiredAngularAccelerationHipJoint(s, &getErrs[0], &getPoses[0]);
    double desAccExoKnee = computeDesiredAngularAccelerationKneeJoint(s, &getErrs[1], &getPoses[1]);
    double desAccExoAnkle = computeDesiredAngularAccelerationAnkleJoint(s, &getErrs[2], &getPoses[2]);
    SimTK::Vector_<double> desAcc(7);
    desAcc[0] = desAccExoHip; desAcc[1] = desAccExoKnee;
    desAcc[2] = desAccExoKnee; desAcc[3] = desAccExoAnkle;
    desAcc[4] = desAccExoAnkle; desAcc[5] = desAccExoKnee;
    desAcc[6] = desAccExoHip;

    SimTK::Vector torques;
    _model->getMatterSubsystem().multiplyByM(s, desAcc, torques);

    return torques;
}

SimTK::Vector PositionController::computeGravityCompensation(const SimTK::State &s) const
{
    SimTK::Vector g;
    _model->getMatterSubsystem().
            multiplyBySystemJacobianTranspose(s,
                                              _model->getGravityForce().getBodyForces(s),
                                              g);
    return g;
}

SimTK::Vector PositionController::computeCoriolisCompensation(const SimTK::State &s) const
{
    SimTK::Vector c;
    _model->getMatterSubsystem().
            calcResidualForceIgnoringConstraints(s,
                                                 SimTK::Vector(0),
                                                 SimTK::Vector_<SpatialVec>(0),
                                                 SimTK::Vector(0), c);
    return c;
}



void PositionController::computeControls(const SimTK::State& s, SimTK::Vector &controls) const
{
   double getErrs[3];
   double getPoses[3];
   SimTK::Vector dynTorque(computeDynamicsTorque(s, getErrs, getPoses));
   SimTK::Vector grav(computeGravityCompensation(s));
   SimTK::Vector coriolis(computeCoriolisCompensation(s));

   const OpenSim::ScalarActuator& ActuatorExoHip = (OpenSim::ScalarActuator&)(_model->getActuators().get("exoHipActuator_r"));
   double nTorqueExoHip = (dynTorque.get(0) + dynTorque.get(6)
                           + coriolis.get(0) + coriolis.get(6)
                           -(grav.get(0) + grav.get(6))
                           ) / ActuatorExoHip.getOptimalForce();
   const OpenSim::ScalarActuator& ActuatorExoKnee = (OpenSim::ScalarActuator&)(_model->getActuators().get("exoKneeActuator_r"));
   double nTorqueExoKnee = (dynTorque.get(1) + dynTorque.get(2) + dynTorque.get(5)
                            + coriolis.get(1) + coriolis.get(2) + coriolis.get(5)
                            -(grav.get(1) + grav.get(2) + grav.get(3))
                            ) / ActuatorExoKnee.getOptimalForce();
   const OpenSim::ScalarActuator& ActuatorExoAnkle = (OpenSim::ScalarActuator&)(_model->getActuators().get("exoAnkleActuator_r"));
   double nTorqueExoAnkle = (dynTorque.get(3) + dynTorque.get(4)
                             + coriolis.get(3) + coriolis.get(4)
                            - (grav.get(3) + grav.get(4))
                             ) / ActuatorExoAnkle.getOptimalForce();
//   for(int i = 0; i < 3; i++)
//       cout << "Error " << i+1 << " = " << getErrs[i] << endl;
   vector<string> nameData = {"Error J1", "Error J2", "Error J3",
                              "Position J1", "Position J2", "Position J3",
                              "desTorque1","desTorque2", "desTorque3",
                              "coriolis1","coriolis2", "coriolis3",
                              "grav1","grav2", "grav3",
                              "time"};
   vector<double> dataToPrint(nameData.size());
   for(int i = 0; i < 3; i++)
   {
       dataToPrint[i] = getErrs[i]; // Save errors
       dataToPrint[3 + i] = getPoses[i]; // Save positions
       dataToPrint[6 + i] = dynTorque.get(i); // Save Dynamics Torques
       dataToPrint[9 + i] = coriolis.get(i); // Save coriolis and centrifugal torques
       dataToPrint[12 + i] = grav.get(i); // Save
       dataToPrint[15] = s.getTime();
    }
   // Print all data to file
   static PrintToFile printFile("DataToPlot.csv");
   printFile.PrintDataToFile(nameData, dataToPrint);
   Vector torqueControl(1, 0.0);
   _model->updActuators().get("exoHipActuator_r").addInControls(torqueControl, controls);
   Vector torqueControl2(1, 0.0);
   _model->updActuators().get("exoKneeActuator_r").addInControls(torqueControl2, controls);
   Vector torqueControl3(1, 0.0);
   _model->updActuators().get("exoAnkleActuator_r").addInControls(torqueControl3, controls);
}



