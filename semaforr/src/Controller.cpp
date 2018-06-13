/*
 * Controller.cpp
 *
 */
          
#include "Controller.h"
#include "FORRGeometry.h"
#include <unistd.h>

#include <deque>
#include <iostream> 
#include <fstream>
#include <math.h>
#include <time.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define CTRL_DEBUG true

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the config file and intialize advisors and weights and spatial learning modules based on the advisors
//
//
void Controller::initialize_advisors(string filename){
 
  string fileLine;
  string advisor_name, advisor_description;
  bool advisor_active;
  double advisor_weight = 1;
  double parameters[4];
  std::ifstream file(filename.c_str());
  ROS_DEBUG_STREAM("Reading read_advisor_file:" << filename);
  if(!file.is_open()){
    ROS_DEBUG("Unable to locate or read advisor config file!");
  }
  //read advisor names and parameters from the config file and create new advisor objects
  while(getline(file, fileLine)){
    if(fileLine[0] == '#')  // skip comment lines
      continue;
    else{
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      advisor_name = vstrings[0];
      advisor_description = vstrings[1];
      if(vstrings[2] == "t")
        advisor_active = true;
      else
        advisor_active = false;
        advisor_weight = atof(vstrings[3].c_str());
        parameters[0]= atof(vstrings[4].c_str());
        parameters[1] = atof(vstrings[5].c_str());
        parameters[2] = atof(vstrings[6].c_str());
        parameters[3] = atof(vstrings[7].c_str());
        tier3Advisors.push_back(Tier3Advisor::makeAdvisor(getBeliefs(), advisor_name, advisor_description, advisor_weight, parameters, advisor_active));
    }
  }
     
  ROS_DEBUG_STREAM("" << tier3Advisors.size() << " advisors registered.");
  for(unsigned i = 0; i < tier3Advisors.size(); ++i)
    ROS_DEBUG_STREAM("Created advisor " << tier3Advisors[i]->get_name() << " with weight: " << tier3Advisors[i]->get_weight());

  //CONVEYORS = isAdvisorActive("ConveyLinear");
  //REGIONS = isAdvisorActive("ExitLinear");
  //TRAILS = isAdvisorActive("TrailerLinear");
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the config file and intialize robot parameters
//
//
void Controller::initialize_params(string filename){
// robot intial position
// robot laser sensor range, span and increment
// robot action <-> semaFORR decision

  string fileLine;
  std::ifstream file(filename.c_str());
  ROS_DEBUG_STREAM("Reading params_file:" << filename);
  //cout << "Inside file in tasks " << endl;
  if(!file.is_open()){
    ROS_DEBUG("Unable to locate or read params config file!");
  }
  while(getline(file, fileLine)){
  //cout << "Inside while in tasks" << endl;
    if(fileLine[0] == '#')  // skip comment lines
      continue;
    else if (fileLine.find("decisionlimit") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      taskDecisionLimit = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("decisionlimit " << taskDecisionLimit);
    }
    else if (fileLine.find("canSeePointEpsilon") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      canSeePointEpsilon = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("canSeePointEpsilon " << canSeePointEpsilon);
    }
    else if (fileLine.find("laserScanRadianIncrement") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      laserScanRadianIncrement = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("laserScanRadianIncrement " << laserScanRadianIncrement);
    }
    else if (fileLine.find("robotFootPrint") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      robotFootPrint = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("robotFootPrint " << robotFootPrint);
    }
    else if (fileLine.find("bufferForRobot") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      robotFootPrintBuffer = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("bufferForRobot " << robotFootPrintBuffer);
    }
    else if (fileLine.find("maxLaserRange") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      maxLaserRange = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("maxLaserRange " << maxLaserRange);
    }
    else if (fileLine.find("maxForwardActionBuffer") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      maxForwardActionBuffer = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("maxForwardActionBuffer " << maxForwardActionBuffer);
    }
    else if (fileLine.find("maxForwardActionSweepAngle") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      maxForwardActionSweepAngle = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("maxForwardActionSweepAngle " << maxForwardActionSweepAngle);
    }
    else if (fileLine.find("trailsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      trailsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("trailsOn " << trailsOn);
    }
    else if (fileLine.find("conveyorsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      conveyorsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("conveyorsOn " << conveyorsOn);
    }
    else if (fileLine.find("regionsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      regionsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("regionsOn " << regionsOn);
    }
    else if (fileLine.find("doorsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      doorsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("doorsOn " << doorsOn);
    }
    else if (fileLine.find("aStarOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      aStarOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("aStarOn " << aStarOn);
    }
    else if (fileLine.find("move") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      moveArrMax = vstrings.size()-1;
      int arr_length = 0;
      for (int i=1; i<vstrings.size(); i++){
        if(arr_length < moveArrMax) {
          arrMove[arr_length++] = (atof(vstrings[i].c_str()));
        }
      }
      for (int i=0; i<moveArrMax; i++) {
        ROS_DEBUG_STREAM("arrMove " << arrMove[i]);
      }
    }
    else if (fileLine.find("rotate") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      rotateArrMax = vstrings.size()-1;
      int arr_length = 0;
      for (int i=1; i<vstrings.size(); i++){
        if(arr_length < rotateArrMax) {
          arrRotate[arr_length++] = (atof(vstrings[i].c_str()));
        }
      }
      for (int i=0; i<rotateArrMax; i++) {
        ROS_DEBUG_STREAM("arrRotate " << arrRotate[i]);
      }
    }
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the map file and intialize planner
//
//
void Controller::initialize_planner(string map_config, string map_dimensions, int &l, int &h){
	string fileLine;
	double p;
	std::ifstream file(map_dimensions.c_str());
  	ROS_DEBUG_STREAM("Reading map dimension file:" << map_dimensions);
  	if(!file.is_open()){
    		ROS_DEBUG("Unable to locate or read map dimensions file!");
  	}
  	while(getline(file, fileLine)){
    		//cout << "Inside while in tasks" << endl;
    		if(fileLine[0] == '#')  // skip comment lines
      			continue;
    		else{
      			std::stringstream ss(fileLine);
      			std::istream_iterator<std::string> begin(ss);
      			std::istream_iterator<std::string> end;
      			std::vector<std::string> vstrings(begin, end);
			ROS_DEBUG("Unable to locate or read map dimensions file!");
      			l = atoi(vstrings[0].c_str());
      			h = atoi(vstrings[1].c_str());
			p = atof(vstrings[2].c_str());
     			ROS_DEBUG_STREAM("Map dim:" << l << " " << h << " " << p << endl);
    		}
  	}	
	Map *map = new Map(l*100, h*100);
	map->readMapFromXML(map_config);
	cout << "Finished reading map"<< endl;
	
	Graph *navGraph = new Graph(map,(int)(p*100.0));
	cout << "initialized nav graph" << endl;
	//navGraph->printGraph();
	//navGraph->outputGraph();
	Node n;
        planner = new PathPlanner(navGraph, *map, n,n);
	cout << "initialized planner" << endl;

  Graph *origNavGraph = new Graph(map,(int)(p*100.0));
  planner->setOriginalNavGraph(origNavGraph);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the config file and intialize tasks
//
//
void Controller::initialize_tasks(string filename){
  string fileLine;
  std::ifstream file(filename.c_str());
  ROS_DEBUG_STREAM("Reading read_task_file:" << filename);
  //cout << "Inside file in tasks " << endl;
  if(!file.is_open()){
    ROS_DEBUG("Unable to locate or read task config file!");
  }
  while(getline(file, fileLine)){
    //cout << "Inside while in tasks" << endl;
    if(fileLine[0] == '#')  // skip comment lines
      continue;
    else{
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      double x = atof(vstrings[0].c_str());
      double y = atof(vstrings[1].c_str());
      beliefs->getAgentState()->addTask(x,y);
      ROS_DEBUG_STREAM("Task:" << x << " " << y << endl);
    }
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize the controller and setup messaging to ROS
//
//
Controller::Controller(string advisor_config, string params_config, string map_config, string target_set, string map_dimensions){

  // Initialize robot parameters from a config file
  initialize_params(params_config);
  
  // Initialize planner and map dimensions
  int l,h;
  initialize_planner(map_config,map_dimensions,l,h);

  // Initialize the agent's 'beliefs' of the world state with the map and nav
  // graph and spatial models
  beliefs = new Beliefs(l, h, 2, arrMove, arrRotate, moveArrMax, rotateArrMax); // Hunter Fourth

  // Initialize advisors and weights from config file
  initialize_advisors(advisor_config);

  // Initialize the tasks from a config file
  initialize_tasks(target_set);

  // Initialize parameters
  beliefs->getAgentState()->setAgentStateParameters(canSeePointEpsilon, laserScanRadianIncrement, robotFootPrint, robotFootPrintBuffer, maxLaserRange, maxForwardActionBuffer, maxForwardActionSweepAngle);
  tier1 = new Tier1Advisor(beliefs);
  firstTaskAssigned = false;
}


// Function which takes sensor inputs and updates it for semaforr to use for decision making, and updates task status
void Controller::updateState(Position current, sensor_msgs::LaserScan laser_scan, geometry_msgs::PoseArray crowdpose, geometry_msgs::PoseArray crowdposeall){
  cout << "inupdate state" << endl;
  beliefs->getAgentState()->setCurrentSensor(current, laser_scan);
  beliefs->getAgentState()->setCrowdPose(crowdpose);
  beliefs->getAgentState()->setCrowdPoseAll(crowdposeall);
  if(firstTaskAssigned == false){
      cout << "Set first task" << endl;
      beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask(),current,planner,aStarOn);
      beliefs->getAgentState()->generateOrigWaypoints(Position(beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_x(), beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_y(),0),planner,aStarOn);
      firstTaskAssigned = true;
  }
  bool waypointReached = beliefs->getAgentState()->getCurrentTask()->isWaypointComplete(current);
  bool taskCompleted = beliefs->getAgentState()->getCurrentTask()->isTaskComplete(current);

  //if task is complete
  if(taskCompleted == true){
    ROS_DEBUG("Target Achieved, moving on to next target!!");
    //Learn spatial model only on tasks completed successfully
    learnSpatialModel(beliefs->getAgentState());
    //Clear existing task and associated plans
    beliefs->getAgentState()->finishTask();
    if(beliefs->getAgentState()->getAgenda().size() > 0){
      //Tasks the next task , current position and a planner and generates a sequence of waypoints if astaron is true
      beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask(),current,planner,aStarOn);
      beliefs->getAgentState()->generateOrigWaypoints(Position(beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_x(), beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_y(),0),planner,aStarOn);
    }
  } 
  // else if subtask is complete
  else if(waypointReached == true and aStarOn){
    ROS_DEBUG("Waypoint reached, but task still incomplete, switching to nearest visible waypoint towards target!!");
    //beliefs->getAgentState()->getCurrentTask()->setupNextWaypoint(current);
    beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getCurrentTask(),current,planner,aStarOn);
    if (beliefs->getAgentState()->getCurrentTask()->getWaypoints().size() > 0) {
      beliefs->getAgentState()->generateOrigWaypoints(Position(beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_x(), beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_y(),0),planner,aStarOn);
    }
    else {
      beliefs->getAgentState()->generateOrigWaypoints(current,planner,aStarOn);
    }
    
  } 
  // otherwise if task Decision limit reached, skip task 
  if(beliefs->getAgentState()->getCurrentTask()->getDecisionCount() > taskDecisionLimit){
    ROS_DEBUG_STREAM("Controller.cpp decisionCount > " << taskDecisionLimit << " , skipping task");
    //learnSpatialModel(beliefs->getAgentState());
    //beliefs->getAgentState()->skipTask();
    beliefs->getAgentState()->finishTask();
    if(beliefs->getAgentState()->getAgenda().size() > 0){
      beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask(),current,planner,aStarOn);
      beliefs->getAgentState()->generateOrigWaypoints(Position(beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_x(), beliefs->getAgentState()->getCurrentTask()->getWaypoints()[0].get_y(),0),planner,aStarOn);
    }
  }
}


// Function which returns the mission status
bool Controller::isMissionComplete(){
  return beliefs->getAgentState()->isMissionComplete();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main robot decision making engine, return decisions that would lead the robot to complete its mission
// Manages switching tasks and stops if the robot is taking too long
//
FORRAction Controller::decide() {
  ROS_DEBUG("Entering decision loop");
  return FORRDecision(); 
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Update spatial model after every task 
//
//

void Controller::learnSpatialModel(AgentState* agentState){
  Task* completedTask = agentState->getCurrentTask();
  vector<Position> *pos_hist = completedTask->getPositionHistory();
  vector< vector<CartesianPoint> > *laser_hist = completedTask->getLaserHistory();
  vector< vector<CartesianPoint> > all_trace = beliefs->getAgentState()->getAllTrace();
 
  if(trailsOn){
    beliefs->getSpatialModel()->getTrails()->updateTrails(agentState);
    beliefs->getSpatialModel()->getTrails()->resetChosenTrail();
  }
  vector< vector<CartesianPoint> > trails_trace = beliefs->getSpatialModel()->getTrails()->getTrailsPoints();
  if(conveyorsOn){
    //beliefs->getSpatialModel()->getConveyors()->populateGridFromPositionHistory(pos_hist);
    beliefs->getSpatialModel()->getConveyors()->populateGridFromTrailTrace(trails_trace.back());
  }
  if(regionsOn){
    beliefs->getSpatialModel()->getRegionList()->learnRegions(pos_hist, laser_hist);
    beliefs->getSpatialModel()->getRegionList()->clearAllExits();
    beliefs->getSpatialModel()->getRegionList()->learnExits(all_trace);
    beliefs->getSpatialModel()->getRegionList()->learnExits(trails_trace);
  }
  vector<FORRRegion> regions = beliefs->getSpatialModel()->getRegionList()->getRegions();
  if(doorsOn){
    beliefs->getSpatialModel()->getDoors()->clearAllDoors();
    beliefs->getSpatialModel()->getDoors()->learnDoors(regions);
  }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SemaFORR decision workflow
//
//
FORRAction Controller::FORRDecision()
{  
  ROS_DEBUG("In FORR decision");
  FORRAction *decision = new FORRAction();
  // Basic semaFORR three tier decision making architecture 
  if(!tierOneDecision(decision)){
  	ROS_DEBUG("Decision to be made by t3!!");
  	//decision->type = FORWARD;
  	//decision->parameter = 5;
  	tierThreeDecision(decision);
  	tierThreeAdvisorInfluence();
  	decisionStats->decisionTier = 3;
  }
  //cout << "decisionTier = " << decisionStats->decisionTier << endl;
  //ROS_DEBUG("After decision made");
  beliefs->getAgentState()->getCurrentTask()->incrementDecisionCount();
  //ROS_DEBUG("After incrementDecisionCount");
  beliefs->getAgentState()->getCurrentTask()->saveDecision(*decision);
  //ROS_DEBUG("After saveDecision");
  beliefs->getAgentState()->clearVetoedActions();
  //ROS_DEBUG("After clearVetoedActions");
  if(decision->type == FORWARD or decision->type == PAUSE){
    beliefs->getAgentState()->setRotateMode(true);
  }
  else{
    beliefs->getAgentState()->setRotateMode(false);
  }

  return *decision;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Generate tier 1 decision
//
//
bool Controller::tierOneDecision(FORRAction *decision){
  //decision making tier1 advisor
  bool decisionMade = false;
  if(tier1->advisorVictory(decision)){ 
	ROS_INFO_STREAM("Advisor victory has made a decision " << decision->type << " " << decision->parameter);
	decisionStats->decisionTier = 1;
	decisionMade = true;	
  }
  else{
  	// group of vetoing tier1 advisors which adds to the list of vetoed actions
	ROS_INFO("Advisor avoid wall will veto actions");
  	tier1->advisorAvoidWalls();
	ROS_INFO("Advisor not opposite will veto actions");
  	tier1->advisorNotOpposite();
  }
  set<FORRAction> *vetoedActions = beliefs->getAgentState()->getVetoedActions();
  std::stringstream vetoList;
  set<FORRAction>::iterator it;
  for(it = vetoedActions->begin(); it != vetoedActions->end(); it++){
    vetoList << it->type << " " << it->parameter << ";";
  }
  decisionStats->vetoedActions = vetoList.str();
  //cout << "vetoedActions = " << vetoList.str() << endl;
  
  return decisionMade;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Generate tier 3 decision
//
//
void Controller::tierThreeDecision(FORRAction *decision){
  std::map<FORRAction, double> comments;
  // This map will aggregate value of all advisers
  std::map<FORRAction, double> allComments;

  // typedef to make for declaration that iterates over map shorter
  typedef map<FORRAction, double>::iterator mapIt;

  // vector of all the actions that got max comment strength in iteration
  vector<FORRAction> best_decisions;
  
  double rotationBaseline, linearBaseline;
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    Tier3Advisor *advisor = *it;
    //if(advisor->is_active() == true)
      //cout << advisor->get_name() << " : " << advisor->get_weight() << endl;
    if(advisor->get_name() == "RotationBaseLine") rotationBaseline = advisor->get_weight();
    if(advisor->get_name() == "BaseLine")         linearBaseline   = advisor->get_weight();
  }
       
  std::stringstream advisorsList;
  std::stringstream advisorCommentsList;
  cout << "processing advisors::"<< endl;
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    Tier3Advisor *advisor = *it; 
    cout << advisor->get_name() << endl;
    // check if advisor should make a decision
    advisor->set_commenting();
    if(advisor->is_active() == false){
      cout << advisor->get_name() << " is inactive " << endl;
      advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";
      continue;
    }
    if(advisor->is_commenting() == false){
      cout << advisor->get_name() << " is not commenting " << endl;
      advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";
      continue;
    }

    advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";

    cout << "Before commenting " << endl;
    comments = advisor->allAdvice();
    cout << "after commenting " << endl;
    // aggregate all comments

    for(mapIt iterator = comments.begin(); iterator != comments.end(); iterator++){
      //cout << "comment : " << (iterator->first.type) << (iterator->first.parameter) << " " << (iterator->second) << endl;
      // If this is first advisor we need to initialize our final map
      float weight;
      //cout << "Agenda size :::::::::::::::::::::::::::::::::: " << beliefs->getAgenda().size() << endl;
      cout << "<" << advisor->get_name() << "," << iterator->first.type << "," << iterator->first.parameter << "> : " << iterator->second << endl; 
      weight = advisor->get_weight();
      cout << "Weight for this advisor : " << weight << endl;

      advisorCommentsList << advisor->get_name() << " " << iterator->first.type << " " << iterator->first.parameter << " " << iterator->second << ";";

      if( allComments.find(iterator->first) == allComments.end()){
	    allComments[iterator->first] =  iterator->second * weight;
      }
      else{
	    allComments[iterator->first] += iterator->second * weight;
      }
    }
  } 
  
  // Loop through map advisor created and find command with the highest vote
  double maxAdviceStrength = -1000;
  for(mapIt iterator = allComments.begin(); iterator != allComments.end(); iterator++){
    //cout << "Values are : " << iterator->first.type << " " << iterator->first.parameter << " with value: " << iterator->second << endl;
    if(iterator->second > maxAdviceStrength){
      maxAdviceStrength = iterator->second;
    }
  }
  //cout << "Max vote strength " << maxAdviceStrength << endl;
  
  for(mapIt iterator = allComments.begin(); iterator!=allComments.end(); iterator++){
    if(iterator->second == maxAdviceStrength)
      best_decisions.push_back(iterator->first);
  }
  
  //cout << "There are " << best_decisions.size() << " decisions that got the highest grade " << endl;
  if(best_decisions.size() == 0){
      (*decision) = FORRAction(PAUSE,0);
  }
  for(unsigned i = 0; i < best_decisions.size(); ++i)
      cout << "Action type: " << best_decisions.at(i).type << " parameter: " << best_decisions.at(i).parameter << endl;
    
  //generate random number using system clock as seed
  srand(time(NULL));
  int random_number = rand() % (best_decisions.size());
    
  (*decision) = best_decisions.at(random_number);
  decisionStats->advisors = advisorsList.str();
  decisionStats->advisorComments = advisorCommentsList.str();
  //cout << " advisors = " << decisionStats->advisors << "\nadvisorComments = " << decisionStats->advisorComments << endl;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Checks if an T3 advisor is active
//
//
bool Controller::
isAdvisorActive(string advisorName){
  bool isActive = false;
  
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    Tier3Advisor *advisor = *it;
    if(advisor->is_active() == true && advisor->get_name() == advisorName)
      isActive = true;
  }
  
  return isActive;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Check influence of tier 3 Advisors
//
//
void Controller::tierThreeAdvisorInfluence(){
  /*std::map<FORRAction, double> comments;
  // This map will aggregate value of all advisers
  std::map<FORRAction, double> allComments;

  // typedef to make for declaration that iterates over map shorter
  typedef map<FORRAction, double>::iterator mapIt;

  // vector of all the actions that got max comment strength in iteration
  vector<FORRAction> best_decisions;
  
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    Tier3Advisor *advisor = *it; 

    // check if advisor should make a decision
    advisor->set_commenting();
    if(advisor->is_active() == false){
      continue;
    }
    if(advisor->is_commenting() == false){
      continue;
    }

    comments = advisor->allAdvice();
    // aggregate all comments

    for(mapIt iterator = comments.begin(); iterator != comments.end(); iterator++){
      // If this is first advisor we need to initialize our final map
      float weight;
      weight = advisor->get_weight();

      if( allComments.find(iterator->first) == allComments.end()){
      allComments[iterator->first] =  iterator->second * weight;
      }
      else{
      allComments[iterator->first] += iterator->second * weight;
      }
    }
  } 
  
  // Loop through map advisor created and find command with the highest vote
  double maxAdviceStrength = -1000;
  for(mapIt iterator = allComments.begin(); iterator != allComments.end(); iterator++){
    cout << "Values are : " << iterator->first.type << " " << iterator->first.parameter << " with value: " << iterator->second << endl;
    if(iterator->second > maxAdviceStrength){
      maxAdviceStrength = iterator->second;
    }
  }
  //cout << "Max vote strength " << maxAdviceStrength << endl;
  
  for(mapIt iterator = allComments.begin(); iterator!=allComments.end(); iterator++){
    if(iterator->second == maxAdviceStrength)
      best_decisions.push_back(iterator->first);
  }
  
  std::stringstream advisorsInfluence;
  std::map<FORRAction, double> takeOneOutComments;
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    takeOneOutComments = allComments;
    Tier3Advisor *advisor = *it; 

    // check if advisor should make a decision
    advisor->set_commenting();
    if(advisor->is_active() == false){
      continue;
    }
    if(advisor->is_commenting() == false){
      advisorsInfluence << advisor->get_name() << " -1;";
      continue;
    }

    comments = advisor->allAdvice();
    // aggregate all comments
    if (comments.size() > 1) {
      for(mapIt iterator = comments.begin(); iterator != comments.end(); iterator++){
        // If this is first advisor we need to initialize our final map
        float weight;
        weight = advisor->get_weight();
        takeOneOutComments[iterator->first]-= iterator->second * weight;
      }
      double maxAdviceStrength = -1000;
      for(mapIt iterator = takeOneOutComments.begin(); iterator != takeOneOutComments.end(); iterator++){
        if(iterator->second > maxAdviceStrength){
          maxAdviceStrength = iterator->second;
        }
      }
      bool same=0;
      for(mapIt iterator = takeOneOutComments.begin(); iterator!=takeOneOutComments.end(); iterator++){
        if(iterator->second == maxAdviceStrength){
          for(unsigned i = 0; i < best_decisions.size(); ++i){
            if(iterator->first.type == best_decisions.at(i).type and iterator->first.parameter == best_decisions.at(i).parameter) {
              same = 0;
            }
            else{
              same = 1;
            }
          }
        }
      }
      if (same == 0){
        advisorsInfluence << advisor->get_name() << " 0;";
      }
      else{
        advisorsInfluence << advisor->get_name() << " 1;";
      }
    }
    else {
      advisorsInfluence << advisor->get_name() << " -1;";
    }
  }
  decisionStats->advisorInfluence = advisorsInfluence.str();
  cout << "advisorInfluence = " << decisionStats->advisorInfluence << endl;*/
  decisionStats->advisorInfluence = "";
}
