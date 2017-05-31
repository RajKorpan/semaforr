
#include <AgentState.h>
#include <iostream>

/*
Position AgentState::getExpectedPositionAfterActions(Position initialPosition, vector<FORRAction> actionList){
  Position currentPosition = initialPosition;
  Position nextPosition;
  for(int i = 0 ; i < actionList.size(); i++){
    FORRAction action = actionList[i];
    nextPosition = getExpectedPositionAfterAction(currentPosition, action);
    // cout << "AgentState :: getExpectedPositionAfterActions >> nextposition " << nextPosition.getX() << " " << nextPosition.getY() << endl;
    currentPosition = nextPosition;
  }
  return nextPosition;
}
*/

Position AgentState::getExpectedPositionAfterAction(FORRAction action){
  Position result; 
  int intensity = action.parameter;
  FORRActionType type = action.type;
  Position initialPosition = getCurrentPosition();
  
  switch(type){
  case FORWARD:
    result = afterLinearMove(initialPosition, move[intensity]);
    break;
  case RIGHT_TURN:
    result = afterAngularMove(initialPosition, -rotate[intensity]);
    break;
  case LEFT_TURN:
    result = afterAngularMove(initialPosition, rotate[intensity]);
    break;
  case PAUSE:
    result = initialPosition;
    break;
  }
  return result;
}
  

Position AgentState::afterLinearMove(Position initialPosition, double distance){
  if(distance > getDistanceToForwardObstacle()) distance = getDistanceToForwardObstacle();

  double new_x = initialPosition.getX() + (distance * cos(initialPosition.getTheta()));
  double new_y = initialPosition.getY() + (distance * sin(initialPosition.getTheta()));
  
  return Position(new_x, new_y, initialPosition.getTheta());
}


Position AgentState::afterAngularMove(Position initialPosition, double angle){

  double new_angle = (angle + initialPosition.getTheta());
  
  double distance = getDistanceToObstacle(angle);
  //max is the maximum look ahead in meters 
  double max = 5;
  if(distance > max) distance = max;

  double new_x = initialPosition.getX() + (distance * cos(new_angle));
  double new_y = initialPosition.getY() + (distance * sin(new_angle));
  
  return Position(new_x, new_y, new_angle);
}


//return the distance to obstacle when the robot is in POS using the current laser scan
double AgentState::getDistanceToNearestObstacle(Position pos){ 
   double min_dis = 1000000;
   for(int i = 0; i < laserEndpoints.size(); i++){
	double x = laserEndpoints[i].get_x();
 	double y = laserEndpoints[i].get_y();
	double dist = pos.getDistance(Position(x,y,0));
	if(dist < min_dis)
		min_dis = dist; 
   }
   return min_dis;
}


//Sees if the laser scan intersects with a segment created by 2
//trailpoints
bool AgentState::canSeeSegment(CartesianPoint point1, CartesianPoint point2){
  CartesianPoint curr(currentPosition.getX(),currentPosition.getY());
  return canSeeSegment(laserEndpoints, curr, point1, point2);
}

//Sees if the laser scan intersects with a segment created by 2
//trailpoints
bool AgentState::canSeeSegment(vector<CartesianPoint> givenLaserEndpoints, CartesianPoint laserPos, CartesianPoint point1, CartesianPoint point2){
  CartesianPoint intersection_point(0,0);
  bool canSeeSegment = false;
  for(int i = 0; i < givenLaserEndpoints.size(); i++){
    //this can be cleaned up or put into one function, but need to convert the distance to a linesegment to calculate
    //the distance from a point to a line
    CartesianPoint endpoint = givenLaserEndpoints[i];
    
    LineSegment distance_vector_line = LineSegment(laserPos, endpoint);
    LineSegment trail_segment = LineSegment(point1, point2);
    if(do_intersect(distance_vector_line, trail_segment, intersection_point)){
      canSeeSegment = true;
	break;
    }
  }
  //else, not visible
  return canSeeSegment;
}


//returns true if there is a point that is "visible" by the wall distance vectors to some epsilon.  
//A point is visible if the distance to a wall distance vector line is < epsilon.
bool AgentState::canSeePoint(vector<CartesianPoint> givenLaserEndpoints, CartesianPoint laserPos, CartesianPoint point){
  ROS_DEBUG_STREAM("AgentState:canSeePoint() , robot pos " << laserPos.get_x() << "," << laserPos.get_y() << " target " <<
	point.get_x() << "," << point.get_y()); 
  double epsilon = 0.2;
  ROS_DEBUG_STREAM("Number of laser endpoints " << givenLaserEndpoints.size()); 
  bool canSeePoint = false;
  for(int i = 0; i < givenLaserEndpoints.size(); i++){
    //ROS_DEBUG_STREAM("Laser endpoint : " << givenLaserEndpoints[i].get_x() << "," << givenLaserEndpoints[i].get_y());
    double ab = laserPos.get_distance(point);
    double ac = laserPos.get_distance(givenLaserEndpoints[i]);
    double bc = givenLaserEndpoints[i].get_distance(point);
    if(((ab + bc) - ac) < epsilon){
      //cout << "Distance vector endpoint visible: ("<<laserEndpoints[i].get_x()<<","<< laserEndpoints[i].get_y()<<")"<<endl; 
      //cout << "Distance: "<<distance_to_point<<endl;
      canSeePoint = true;
      break;
    }
  }
  //else, not visible
  return canSeePoint;
}



//returns true if there is a point that is "visible" by the wall distance vectors to some epsilon.  
//A point is visible if the distance to a wall distance vector line is < epsilon.
bool AgentState::canSeePoint(CartesianPoint point){
  CartesianPoint curr(currentPosition.getX(),currentPosition.getY());
  return canSeePoint(laserEndpoints, curr, point);
}

void AgentState::transformToEndpoints(){
    ROS_DEBUG("Convert laser scan to endpoints");
    double start_angle = currentLaserScan.angle_min;
    double increment = currentLaserScan.angle_increment;
    laserEndpoints.clear();

    double r_x = currentPosition.getX();
    double r_y = currentPosition.getY();
    double r_ang = currentPosition.getTheta();
    CartesianPoint current_point(r_x,r_y);
  
    for(int i = 0 ; i < currentLaserScan.ranges.size(); i++){
       Vector v = Vector(current_point, start_angle + r_ang, currentLaserScan.ranges[i]);
       CartesianPoint endpoint = v.get_endpoint();
       start_angle = start_angle + increment;
       laserEndpoints.push_back(endpoint);
    }    
}

double AgentState::getDistanceToObstacle(double rotation_angle){
	ROS_DEBUG("In getDistanceToObstacle");
	// one increment in the laser range scan is 1/3 degrees, i.e 0.005817 in radians  
	int index = (int)(rotation_angle/(0.005817));
	//ROS_DEBUG("In getDistanceToObstacle after index");
	//shift the index in the positive
	index = index + 330;
	//ROS_DEBUG("In getDistanceToObstacle after index shift");
	if(index < 0) index = 0;
	//ROS_DEBUG("In getDistanceToObstacle after first if");
	if(index > 659) index = 659;
	//ROS_DEBUG("In getDistanceToObstacle after second if");
	//cout << index << " " << currentLaserScan.ranges.size() << endl;
	if(currentLaserScan.ranges.size() == 0) { return 25; }
	//cout << currentLaserScan.ranges[index] << endl;
	return currentLaserScan.ranges[index];
}


FORRAction AgentState::maxForwardAction(){
 	ROS_DEBUG("In maxforwardaction");
	double error_margin = 1; //one meter margin from the wall
	double forward_distance = getDistanceToForwardObstacle() - error_margin;
	double forward_distance_left = getDistanceToObstacle(0.25) - error_margin;
	double forward_distance_right = getDistanceToObstacle(-0.25) - error_margin;

 	ROS_DEBUG_STREAM("Distances " << forward_distance << "," << forward_distance_left << "," << forward_distance_right);

	double min_distance;
	if(forward_distance_left >= forward_distance_right) min_distance = forward_distance_right;
 	else min_distance = forward_distance_left;

	if(forward_distance < min_distance) min_distance = forward_distance;

	ROS_DEBUG_STREAM("Forward obstacle distance " << min_distance);
	if(min_distance > move[5])
		return FORRAction(FORWARD,5);
	else if(min_distance > move[4] && min_distance <= move[5])
		return FORRAction(FORWARD,4);
	else if(min_distance > move[3] && min_distance <= move[4])
		return FORRAction(FORWARD,3);
	else if(min_distance > move[2] && min_distance <= move[3])
		return FORRAction(FORWARD,2);
	else if(min_distance > move[1] && min_distance <= move[2])
		return FORRAction(FORWARD,1);
	else
		return FORRAction(FORWARD,0);

}


FORRAction AgentState::get_max_allowed_forward_move(){
  FORRAction max_forward(FORWARD, 5);
  cout << " Number of vetoed actions : " << vetoedActions->size() << endl;
  for(int intensity = 1; intensity <= 5; intensity++){
    if(vetoedActions->find(FORRAction(FORWARD,intensity)) != vetoedActions->end()){
      max_forward.type = FORWARD;
      max_forward.parameter = intensity - 1;
      break;
    }
  }
  return max_forward;
}


// returns an Action that takes the robot closest to the target
FORRAction AgentState::moveTowards(){
    ROS_DEBUG("AgentState :: In moveTowards");
    double distance_from_target = currentPosition.getDistance(currentTask->getX(), currentTask->getY());
    ROS_DEBUG_STREAM("Distance from target : " << distance_from_target);
    // compute the angular difference between the direction to the target and the current robot direction
    double robot_direction = currentPosition.getTheta();
    double goal_direction = atan2((currentTask->getY() - currentPosition.getY()), (currentTask->getX() - currentPosition.getX()));
    
    double required_rotation = goal_direction - robot_direction;

    ROS_DEBUG_STREAM("Robot direction : " << robot_direction << ", Goal Direction : " << goal_direction << ", Required rotation : " << required_rotation);
    if(required_rotation > 3.1416)
      required_rotation = required_rotation - 6.283;
    if(required_rotation < -3.1416)
      required_rotation = required_rotation + 6.283;
    //cout << "Robot direction : " << robot_direction << ", Goal Direction : " << goal_direction << ", Required rotation : " << required_rotation << endl;
    ROS_DEBUG_STREAM("Robot direction : " << robot_direction << ", Goal Direction : " << goal_direction << ", Required rotation : " << required_rotation);
    // if the angular difference is greater than smallest turn possible 
    // pick the right turn to allign itself to the target
    
    FORRAction decision;

    if(fabs(required_rotation) > rotate[1]){
      if( required_rotation > rotate[1] && required_rotation <= rotate[2])
	decision = FORRAction(LEFT_TURN, 1);
      else if( required_rotation > rotate[2] && required_rotation <= rotate[3])
	decision = FORRAction(LEFT_TURN, 2);
      else if(required_rotation > rotate[3] && required_rotation <= rotate[4])
	decision = FORRAction(LEFT_TURN, 3);
      else if(required_rotation > rotate[4])
	decision = FORRAction(LEFT_TURN, 4);
      else if( required_rotation < -rotate[1] && required_rotation >= -rotate[2])
	decision = FORRAction(RIGHT_TURN, 1);
      else if( required_rotation < -rotate[2] && required_rotation >= -rotate[3])
	decision = FORRAction(RIGHT_TURN, 2);
      else if(required_rotation < -rotate[3] && required_rotation >= -rotate[4])
	decision = FORRAction(RIGHT_TURN, 3);
      else if(required_rotation < -rotate[4])
	decision = FORRAction(RIGHT_TURN, 4);
    }
    else{
      int intensity;
      if(distance_from_target <= move[1])
	intensity = 0;
      else if(distance_from_target <= move[2])
	intensity = 1;
      else if(distance_from_target <= move[3])
	intensity = 2;
      else if(distance_from_target <= move[4])
	intensity = 3;
      else if(distance_from_target <= move[5])
	intensity = 4;
      else
	intensity = 5;
      decision = FORRAction(FORWARD,intensity);
    }
    ROS_INFO_STREAM("Action choosen : " << decision.type << "," << decision.parameter);
    return decision;
}



