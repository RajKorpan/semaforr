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
      taskDecisionLimit = atoi(vstrings[1].c_str());
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
    else if (fileLine.find("highwayDistanceThreshold") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      highwayDistanceThreshold = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("highwayDistanceThreshold " << highwayDistanceThreshold);
    }
    else if (fileLine.find("highwayTimeThreshold") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      highwayTimeThreshold = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("highwayTimeThreshold " << highwayTimeThreshold);
    }
    else if (fileLine.find("highwayDecisionThreshold") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      highwayDecisionThreshold = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("highwayDecisionThreshold " << highwayDecisionThreshold);
    }
    else if (fileLine.find("maxForwardActionSweepAngle") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      maxForwardActionSweepAngle = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("maxForwardActionSweepAngle " << maxForwardActionSweepAngle);
    }
    else if (fileLine.find("planLimit") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      planLimit = atoi(vstrings[1].c_str());
      ROS_DEBUG_STREAM("planLimit " << planLimit);
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
    else if (fileLine.find("hallwaysOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      hallwaysOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("hallwaysOn " << hallwaysOn);
    }
    else if (fileLine.find("barrsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      barrsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("barrsOn " << barrsOn);
    }
    else if (fileLine.find("situationsOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      situationsOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("situationsOn " << situationsOn);
    }
    else if (fileLine.find("highwaysOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      highwaysOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("highwaysOn " << highwaysOn);
    }
    else if (fileLine.find("outofhereOn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      outofhereOn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("outofhereOn " << outofhereOn);
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
    else if (fileLine.find("distance") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      distance = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("distance " << distance);
    }
    else if (fileLine.find("smooth") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      smooth = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("smooth " << smooth);
    }
    else if (fileLine.find("novel") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      novel = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("novel " << novel);
    }
    else if (fileLine.find("density") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      density = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("density " << density);
    }
    else if (fileLine.find("risk") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      risk = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("risk " << risk);
    }
    else if (fileLine.find("flow") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      flow = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("flow " << flow);
    }
    else if (fileLine.find("combined") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      combined = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("combined " << combined);
    }
    else if (fileLine.find("CUSUM") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      CUSUM = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("CUSUM " << CUSUM);
    }
    else if (fileLine.find("discount") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      discount = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("discount " << discount);
    }
    else if (fileLine.find("explore") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      explore = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("explore " << explore);
    }
    else if (fileLine.find("spatial") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      spatial = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("spatial " << spatial);
    }
    else if (fileLine.find("hallwayer") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      hallwayer = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("hallwayer " << hallwayer);
    }
    else if (fileLine.find("trailer") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      trailer = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("trailer " << trailer);
    }
    else if (fileLine.find("barrier") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      barrier = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("barrier " << barrier);
    }
    else if (fileLine.find("conveys") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      conveys = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("conveys " << conveys);
    }
    else if (fileLine.find("turn") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      turn = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("turn " << turn);
    }
    else if (fileLine.find("skeleton") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      skeleton = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("skeleton " << skeleton);
    }
    else if (fileLine.find("hallwayskel") != std::string::npos) {
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      hallwayskel = atof(vstrings[1].c_str());
      ROS_DEBUG_STREAM("hallwayskel " << hallwayskel);
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
  Graph *origNavGraph = new Graph(map,(int)(p*100.0));
  //Graph *navGraph = new Graph(map,(int)(p*100.0));
  //cout << "initialized nav graph" << endl;
  //navGraph->printGraph();
  //navGraph->outputGraph();
  Node n;
  if(distance == 1){
    Graph *navGraphDistance = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphDistance, *map, n,n, "distance");
    //tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: distance");
  }
  if(smooth == 1){
    Graph *navGraphSmooth = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphSmooth, *map, n,n, "smooth");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: smooth");
  }
  if(novel == 1){
    Graph *navGraphNovel = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphNovel, *map, n,n, "novel");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: novel");
  }
  if(density == 1){
    Graph *navGraphDensity = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphDensity, *map, n,n, "density");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: density");
  }
  if(risk == 1){
    Graph *navGraphRisk = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphRisk, *map, n,n, "risk");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: risk");
  }
  if(flow == 1){
    Graph *navGraphFlow = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphFlow, *map, n,n, "flow");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: flow");
  }
  if(combined == 1){
    Graph *navGraphCombined = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphCombined, *map, n,n, "combined");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: combined");
  }
  if(CUSUM == 1){
    Graph *navGraphCUSUM = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphCUSUM, *map, n,n, "CUSUM");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: CUSUM");
  }
  if(discount == 1){
    Graph *navGraphDiscount = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphDiscount, *map, n,n, "discount");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: discount");
  }
  if(explore == 1){
    Graph *navGraphExplore = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphExplore, *map, n,n, "explore");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: explore");
  }
  if(spatial == 1){
    Graph *navGraphSpatial = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphSpatial, *map, n,n, "spatial");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: spatial");
  }
  if(hallwayer == 1){
    Graph *navGraphHallwayer = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphHallwayer, *map, n,n, "hallwayer");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: hallwayer");
  }
  if(trailer == 1){
    Graph *navGraphTrailer = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphTrailer, *map, n,n, "trailer");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: trailer");
  }
  if(barrier == 1){
    Graph *navGraphBarrier = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphBarrier, *map, n,n, "barrier");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: barrier");
  }
  if(conveys == 1){
    Graph *navGraphConveys = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphConveys, *map, n,n, "conveys");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: conveys");
  }
  if(turn == 1){
    Graph *navGraphTurn = new Graph(map,(int)(p*100.0));
    cout << "initialized nav graph" << endl;
    planner = new PathPlanner(navGraphTurn, *map, n,n, "turn");
    tier2Planners.push_back(planner);
    planner->setOriginalNavGraph(origNavGraph);
    ROS_DEBUG_STREAM("Created planner: turn");
  }
  if(skeleton == 1){
    Graph *navGraphSkeleton = new Graph((int)(p*100.0), l*100, h*100);
    cout << "initialized nav graph" << endl;
    PathPlanner *sk_planner = new PathPlanner(navGraphSkeleton, n,n, "skeleton");
    tier2Planners.push_back(sk_planner);
    Graph *origNavGraphSkeleton = new Graph((int)(p*100.0), l*100, h*100);
    sk_planner->setOriginalNavGraph(origNavGraphSkeleton);
    ROS_DEBUG_STREAM("Created planner: skeleton");
  }
  if(hallwayskel == 1){
    Graph *navGraphHallwaySkeleton = new Graph((int)(p*100.0), l*100, h*100);
    cout << "initialized nav graph" << endl;
    PathPlanner *hwsk_planner = new PathPlanner(navGraphHallwaySkeleton, n,n, "hallwayskel");
    tier2Planners.push_back(hwsk_planner);
    Graph *origNavGraphHallwaySkeleton = new Graph((int)(p*100.0), l*100, h*100);
    hwsk_planner->setOriginalNavGraph(origNavGraphHallwaySkeleton);
    ROS_DEBUG_STREAM("Created planner: hallwayskel");
  }
  cout << "initialized planners" << endl;
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
      ROS_DEBUG_STREAM("Task: " << x << " " << y << endl);
    }
  }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the config file and intialize situations
//
//
void Controller::initialize_situations(string filename){
  string fileLine;
  std::ifstream file(filename.c_str());
  ROS_DEBUG_STREAM("Reading read_situation_file:" << filename);
  //cout << "Inside file in situations " << endl;
  if(!file.is_open()){
    ROS_DEBUG("Unable to locate or read situation config file!");
  }
  while(getline(file, fileLine)){
    //cout << "Inside while in situations" << endl;
    if(fileLine[0] == '#')  // skip comment lines
      continue;
    else{
      std::stringstream ss(fileLine);
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> vstrings(begin, end);
      int count = atoi(vstrings[0].c_str());
      vector<float> values;
      for (int i=1; i<vstrings.size(); i++){
        values.push_back(atof(vstrings[i].c_str()));
      }
      beliefs->getSpatialModel()->getSituations()->createSituations(count, values);
      ROS_DEBUG_STREAM("Situation: " << count << endl);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read from the config file and intialize spatial model
//
//
void Controller::initialize_spatial_model(string filename){
  string fileLine;
  std::ifstream file(filename.c_str());
  ROS_DEBUG_STREAM("Reading read_spatial_model_file:" << filename);
  //cout << "Inside file in spatial model " << endl;
  if(!file.is_open()){
    ROS_DEBUG("Unable to locate or read spatial model config file!");
  }
  while(getline(file, fileLine)){
    //cout << "Inside while in spatial model" << endl;
    if(fileLine[0] == '#')  // skip comment lines
      continue;
    else if (fileLine.find("regions") != std::string::npos) {
      const char delim = ';';
      vector<string> out;
      stringstream ss(fileLine);
      string s;
      while(getline(ss, s, delim)){
        out.push_back(s);
        cout << s << endl;
      }
      vector<FORRRegion> initial_regions;
      vector < vector<CartesianPoint> > traces;
      for(int i = 0; i < out.size(); i++){
        stringstream sst(out[i]);
        istream_iterator<string> begin(sst);
        istream_iterator<string> end;
        vector<string> vstrings(begin, end);
        FORRRegion new_region;
        if(vstrings[0] == "regions"){
          new_region = FORRRegion(CartesianPoint(atof(vstrings[1].c_str()),atof(vstrings[2].c_str())),atof(vstrings[3].c_str()));
          for (int j = 4; j < vstrings.size(); j += 9){
            vector<CartesianPoint> path;
            path.push_back(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())));
            path.push_back(CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())));
            path.push_back(CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())));
            FORRExit new_exit = FORRExit(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())), CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())), CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())), atoi(vstrings[j+2].c_str()), atof(vstrings[j+7].c_str()), atoi(vstrings[j+8].c_str()), path);
            new_region.addExit(new_exit);
          }
        }
        else{
          new_region = FORRRegion(CartesianPoint(atof(vstrings[0].c_str()),atof(vstrings[1].c_str())),atof(vstrings[2].c_str()));
          for (int j = 3; j < vstrings.size(); j += 9){
            vector<CartesianPoint> path;
            path.push_back(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())));
            path.push_back(CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())));
            path.push_back(CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())));
            FORRExit new_exit = FORRExit(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())), CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())), CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())), atoi(vstrings[j+2].c_str()), atof(vstrings[j+7].c_str()), atoi(vstrings[j+8].c_str()), path);
            new_region.addExit(new_exit);
          }
        }
        initial_regions.push_back(new_region);
      }
      for(int i = 0; i < out.size(); i++){
        stringstream sst(out[i]);
        istream_iterator<string> begin(sst);
        istream_iterator<string> end;
        vector<string> vstrings(begin, end);
        if(vstrings[0] == "regions"){
          for (int j = 4; j < vstrings.size(); j += 9){
            vector<CartesianPoint> exit_trace;
            exit_trace.push_back(CartesianPoint(atof(vstrings[1].c_str()),atof(vstrings[2].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())));
            exit_trace.push_back(initial_regions[atoi(vstrings[j+2].c_str())].getCenter());
            traces.push_back(exit_trace);
          }
        }
        else{
          for (int j = 3; j < vstrings.size(); j += 9){
            vector<CartesianPoint> exit_trace;
            exit_trace.push_back(CartesianPoint(atof(vstrings[0].c_str()),atof(vstrings[1].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())));
            exit_trace.push_back(CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())));
            exit_trace.push_back(initial_regions[atoi(vstrings[j+2].c_str())].getCenter());
            traces.push_back(exit_trace);
          }
        }
      }
      beliefs->getSpatialModel()->getRegionList()->setRegions(initial_regions);
      beliefs->getAgentState()->setInitialExitTraces(traces);
      ROS_DEBUG_STREAM("regions " << initial_regions.size());
      beliefs->getSpatialModel()->getDoors()->learnDoors(initial_regions);
      updateSkeletonGraph(beliefs->getAgentState());
    }
  //   else if (fileLine.find("doors") != std::string::npos) {
  //     string str = fileline;
  //     const char delim = ';';
  //     vector<string> out;
  //     stringstream ss(str);
  //     string s;
  //     while(getline(ss, s, delim)){
  //       out.push_back(s);
  //       cout << s << endl;
  //     }
  //     std::vector< std::vector<Door> > initial_doors;


  //     for(int i = 0; i < out.size(); i++){
  //       stringstream sst(out[i]);
  //       istream_iterator<string> begin(sst);
  //       istream_iterator<string> end;
  //       vector<string> vstrings(begin, end);
  //       FORRRegion new_region;
  //       if(vstrings[0] == "regions"){
  //         new_region = FORRRegion(CartesianPoint(atof(vstrings[1].c_str()),atof(vstrings[2].c_str())),atof(vstrings[3].c_str()));
  //         for (int j = 4; j < vstrings.size(); j += 8){
  //           FORRExit new_exit = FORRExit(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())), CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())), CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())), atoi(vstrings[j+2].c_str()), atof(vstrings[j+7].c_str()));
  //           new_region.addExit(new_exit);
  //         }
  //       }
  //       else{
  //         new_region = FORRRegion(CartesianPoint(atof(vstrings[0].c_str()),atof(vstrings[1].c_str())),atof(vstrings[2].c_str()));
  //         for (int j = 3; j < vstrings.size(); j += 8){
  //           FORRExit new_exit = FORRExit(CartesianPoint(atof(vstrings[j].c_str()),atof(vstrings[j+1].c_str())), CartesianPoint(atof(vstrings[j+3].c_str()),atof(vstrings[j+4].c_str())), CartesianPoint(atof(vstrings[j+5].c_str()),atof(vstrings[j+6].c_str())), atoi(vstrings[j+2].c_str()), atof(vstrings[j+7].c_str()));
  //           new_region.addExit(new_exit);
  //         }
  //       }
  //       initial_regions.push_back(new_region);
  //     }
  //     beliefs->getSpatialModel()->getDoors()->setDoors(initial_doors);
  //     ROS_DEBUG_STREAM("doors " << initial_doors.size());
  //   }
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize the controller and setup messaging to ROS
//
//
Controller::Controller(string advisor_config, string params_config, string map_config, string target_set, string map_dimensions, string situation_config, string spatial_model_config){

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
  decisionStats = new FORRActionStats();

  // Initialize situations
  // initialize_situations(situation_config);

  // Initialize spatial model
  // initialize_spatial_model(spatial_model_config);

  // Initialize highways
  highwayFinished = 0;
  highwayExploration = new HighwayExplorer(l, h, highwayDistanceThreshold, highwayTimeThreshold, highwayDecisionThreshold, arrMove, arrRotate, moveArrMax, rotateArrMax);

  // Initialize circumnavigator
  // PathPlanner *skeleton_planner;
  // for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
  //   if((*it)->getName() == "skeleton"){
  //     skeleton_planner = *it;
  //   }
  // }
  // circumnavigator = new Circumnavigate(l, h, arrMove, arrRotate, moveArrMax, rotateArrMax, beliefs, skeleton_planner);
}


// Function which takes sensor inputs and updates it for semaforr to use for decision making, and updates task status
void Controller::updateState(Position current, sensor_msgs::LaserScan laser_scan, geometry_msgs::PoseArray crowdpose, geometry_msgs::PoseArray crowdposeall){
  cout << "In update state" << endl;
  beliefs->getAgentState()->setCurrentSensor(current, laser_scan);
  // beliefs->getSpatialModel()->getSituations()->addObservationToSituations(laser_scan, current, true);
  beliefs->getAgentState()->setCrowdPose(crowdpose);
  beliefs->getAgentState()->setCrowdPoseAll(crowdposeall);
  if(firstTaskAssigned == false){
      cout << "Set first task" << endl;
      if(aStarOn and (!highwaysOn or (highwaysOn and highwayExploration->getHighwaysComplete()))){
        tierTwoDecision(current);
      }
      else{
        beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask());
      }
      firstTaskAssigned = true;
  }
  if(highwayExploration->getHighwaysComplete() or !highwaysOn){
    //bool waypointReached = beliefs->getAgentState()->getCurrentTask()->isWaypointComplete(current);
    bool waypointReached = beliefs->getAgentState()->getCurrentTask()->isAnyWaypointComplete(current, beliefs->getAgentState()->getCurrentLaserEndpoints());
    bool taskCompleted = beliefs->getAgentState()->getCurrentTask()->isTaskComplete(current);
    bool isPlanActive = beliefs->getAgentState()->getCurrentTask()->getIsPlanActive();
    // cout << "waypointReached " <<   waypointReached << " taskCompleted " << taskCompleted << " isPlanActive " << isPlanActive << endl;
    if(highwayFinished == 1){
      if(highwaysOn){
        learnSpatialModel(beliefs->getAgentState(), true);
        ROS_DEBUG("Finished Learning Spatial Model!!");
        updateSkeletonGraph(beliefs->getAgentState());
        ROS_DEBUG("Finished Updating Skeleton Graph!!");
      }
      beliefs->getAgentState()->finishTask();
      ROS_DEBUG("Selecting Next Task");
      if(aStarOn){
        tierTwoDecision(current);
        ROS_DEBUG("Next Plan Generated!!");
      }
      else{
        beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask());
        ROS_DEBUG("Next Task Selected!!");
      }
      beliefs->getAgentState()->setGetOutTriggered(false);
      // beliefs->getAgentState()->resetDirections();
      // circumnavigator->resetCircumnavigate();
    }
    //if task is complete
    if(taskCompleted == true){
      ROS_DEBUG("Target Achieved, moving on to next target!!");
      //Learn spatial model only on tasks completed successfully
      if(beliefs->getAgentState()->getAllAgenda().size() - beliefs->getAgentState()->getAgenda().size() <= 2000){
        learnSpatialModel(beliefs->getAgentState(), true);
        ROS_DEBUG("Finished Learning Spatial Model!!");
        updateSkeletonGraph(beliefs->getAgentState());
        ROS_DEBUG("Finished Updating Skeleton Graph!!");
        if(situationsOn){
          beliefs->getSpatialModel()->getSituations()->learnSituationActions(beliefs->getAgentState(), beliefs->getSpatialModel()->getTrails()->getTrail(beliefs->getSpatialModel()->getTrails()->getSize()-1));
          ROS_DEBUG("Finished Learning Situations!!");
        }
      }
      beliefs->getAgentState()->setGetOutTriggered(false);
      // beliefs->getAgentState()->resetDirections();
      // circumnavigator->resetCircumnavigate();
      //Clear existing task and associated plans
      beliefs->getAgentState()->finishTask();
      //ROS_DEBUG("Task Cleared!!");
      //cout << "Agenda Size = " << beliefs->getAgentState()->getAgenda().size() << endl;
      if(beliefs->getAgentState()->getAgenda().size() > 0){
        //Tasks the next task , current position and a planner and generates a sequence of waypoints if astaron is true
        ROS_DEBUG_STREAM("Controller.cpp taskCount > " << (beliefs->getAgentState()->getAllAgenda().size() - beliefs->getAgentState()->getAgenda().size()) << " planLimit " << (planLimit - 1));
        if((beliefs->getAgentState()->getAllAgenda().size() - beliefs->getAgentState()->getAgenda().size()) > (planLimit - 1)){
          aStarOn = false;
        }
        ROS_DEBUG("Selecting Next Task");
        if(aStarOn){
          tierTwoDecision(current);
          ROS_DEBUG("Next Plan Generated!!");
        }
        else{
          beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask());
          ROS_DEBUG("Next Task Selected!!");
        }
      }
    }
    // else if subtask is complete
    else if(waypointReached == true and beliefs->getAgentState()->getCurrentTask()->getPlanSize() > 0){
      ROS_DEBUG("Waypoint reached, but task still incomplete, switching to nearest visible waypoint towards target!!");
      //beliefs->getAgentState()->getCurrentTask()->setupNextWaypoint(current);
      beliefs->getAgentState()->getCurrentTask()->setupNearestWaypoint(current, beliefs->getAgentState()->getCurrentLaserEndpoints());
      //beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getCurrentTask(),current,planner,aStarOn);
    }
    // else if(isPlanActive == false and aStarOn){
    //   ROS_DEBUG("No active plan, setting up new plan!!");
    //   tierTwoDecision(current);
    // }
    // else if(waypointReached == true and beliefs->getAgentState()->getCurrentTask()->getWaypoints().size() == 1){
    //   ROS_DEBUG("Temporary Waypoint reached!!");
    //   beliefs->getAgentState()->getCurrentTask()->setIsPlanActive(false);
    //   beliefs->getAgentState()->getCurrentTask()->clearWaypoints();
    // }
    // otherwise if task Decision limit reached, skip task 
    if(beliefs->getAgentState()->getCurrentTask() != NULL){
      if(beliefs->getAgentState()->getCurrentTask()->getDecisionCount() > taskDecisionLimit){
        ROS_DEBUG_STREAM("Controller.cpp decisionCount > " << taskDecisionLimit << " , skipping task");
        beliefs->getAgentState()->setGetOutTriggered(false);
        // beliefs->getAgentState()->resetDirections();
        // circumnavigator->resetCircumnavigate();
        learnSpatialModel(beliefs->getAgentState(), false);
        ROS_DEBUG("Finished Learning Spatial Model!!");
        updateSkeletonGraph(beliefs->getAgentState());
        ROS_DEBUG("Finished Updating Skeleton Graph!!");
        if(situationsOn){
          beliefs->getSpatialModel()->getSituations()->learnSituationActions(beliefs->getAgentState(), beliefs->getSpatialModel()->getTrails()->getTrail(beliefs->getSpatialModel()->getTrails()->getSize()-1));
          ROS_DEBUG("Finished Learning Situations!!");
        }
        //beliefs->getAgentState()->skipTask();
        // if(beliefs->getAgentState()->getAllAgenda().size() < planLimit +1){
        //   beliefs->getAgentState()->addTask(beliefs->getAgentState()->getCurrentTask()->getTaskX(),beliefs->getAgentState()->getCurrentTask()->getTaskY());
        // }
        beliefs->getAgentState()->finishTask();
        if(beliefs->getAgentState()->getAgenda().size() > 0){
          ROS_DEBUG_STREAM("Controller.cpp taskCount > " << (beliefs->getAgentState()->getAllAgenda().size() - beliefs->getAgentState()->getAgenda().size()) << " planLimit " << (planLimit - 1));
          if((beliefs->getAgentState()->getAllAgenda().size() - beliefs->getAgentState()->getAgenda().size()) > (planLimit - 1)){
            aStarOn = false;
          }
          if(aStarOn){
            tierTwoDecision(current);
          }
          else{
            beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask());
          }
        }
      }
    }
  }
  
  ROS_DEBUG("End Of UpdateState");
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
  FORRAction decidedAction;
  if(!highwayExploration->getHighwaysComplete() and highwaysOn){
    decidedAction = highwayExploration->exploreDecision(beliefs->getAgentState()->getCurrentPosition(), beliefs->getAgentState()->getCurrentLaserScan());
  }
  else{
    highwayFinished++;
    decidedAction = FORRDecision();
  }
  if(situationsOn){
    beliefs->getSpatialModel()->getSituations()->addObservationToSituations(beliefs->getAgentState()->getCurrentLaserScan(), beliefs->getAgentState()->getCurrentPosition(), true, decidedAction);
  }
  return decidedAction;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Update spatial model after every task 
//
//

void Controller::learnSpatialModel(AgentState* agentState, bool taskStatus){
  double computationTimeSec=0.0;
  timeval cv;
  double start_timecv;
  double end_timecv;
  gettimeofday(&cv,NULL);
  start_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);

  Task* completedTask = agentState->getCurrentTask();
  vector<Position> *pos_hist = completedTask->getPositionHistory();
  vector< vector<CartesianPoint> > *laser_hist = completedTask->getLaserHistory();
  vector< vector<CartesianPoint> > all_trace = beliefs->getAgentState()->getAllTrace();
  // vector< vector<CartesianPoint> > exit_traces = beliefs->getAgentState()->getInitialExitTraces();
  // vector< vector<CartesianPoint> > all_laser_hist = beliefs->getAgentState()->getAllLaserHistory();
  vector< vector < vector<CartesianPoint> > > all_laser_trace = beliefs->getAgentState()->getAllLaserTrace();
  vector<CartesianPoint> trace;
  for(int i = 0 ; i < pos_hist->size() ; i++){
    trace.push_back(CartesianPoint((*pos_hist)[i].getX(),(*pos_hist)[i].getY()));
  }
  all_trace.push_back(trace);
  // for(int i = 0; i < exit_traces.size(); i++){
  //   all_trace.insert(all_trace.begin(), exit_traces[i]);
  // }
  vector < vector<CartesianPoint> > laser_trace;
  for(int i = 0 ; i < laser_hist->size() ; i++){
    laser_trace.push_back((*laser_hist)[i]);
  }
  all_laser_trace.push_back(laser_trace);

  if(trailsOn){
    beliefs->getSpatialModel()->getTrails()->updateTrails(agentState);
    beliefs->getSpatialModel()->getTrails()->resetChosenTrail();
    ROS_DEBUG("Trails Learned");
  }
  vector< vector<CartesianPoint> > trails_trace = beliefs->getSpatialModel()->getTrails()->getTrailsPoints();
  if(conveyorsOn and taskStatus){
    //beliefs->getSpatialModel()->getConveyors()->populateGridFromPositionHistory(pos_hist);
    beliefs->getSpatialModel()->getConveyors()->populateGridFromTrailTrace(trails_trace.back());
    ROS_DEBUG("Conveyors Learned");
  }
  if(regionsOn){
    beliefs->getSpatialModel()->getRegionList()->learnRegionsAndExits(pos_hist, laser_hist, all_trace, all_laser_trace);
    // beliefs->getSpatialModel()->getRegionList()->learnRegions(pos_hist, laser_hist);
    ROS_DEBUG("Regions Learned");
    // beliefs->getSpatialModel()->getRegionList()->clearAllExits();
    // beliefs->getSpatialModel()->getRegionList()->learnExits(all_trace);
    // beliefs->getSpatialModel()->getRegionList()->learnExits(trails_trace);
    ROS_DEBUG("Exits Learned");
  }
  vector<FORRRegion> regions = beliefs->getSpatialModel()->getRegionList()->getRegions();
  if(doorsOn){
    beliefs->getSpatialModel()->getDoors()->clearAllDoors();
    beliefs->getSpatialModel()->getDoors()->learnDoors(regions);
    ROS_DEBUG("Doors Learned");
  }
  if(hallwaysOn){
    //beliefs->getSpatialModel()->getHallways()->clearAllHallways();
    //beliefs->getSpatialModel()->getHallways()->learnHallways(agentState, all_trace, all_laser_hist);
    beliefs->getSpatialModel()->getHallways()->learnHallways(agentState, trace, laser_hist);
    //beliefs->getSpatialModel()->getHallways()->learnHallways(trails_trace);
    ROS_DEBUG("Hallways Learned");
  }
  if(barrsOn){
    beliefs->getSpatialModel()->getBarriers()->updateBarriers(laser_hist, all_trace.back());
    ROS_DEBUG("Barriers Learned");
  }
  gettimeofday(&cv,NULL);
  end_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
  computationTimeSec = (end_timecv-start_timecv);
  decisionStats->learningComputationTime = computationTimeSec;
}

void Controller::updateSkeletonGraph(AgentState* agentState){
  double computationTimeSec=0.0;
  timeval cv;
  double start_timecv;
  double end_timecv;
  gettimeofday(&cv,NULL);
  start_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);

  if((skeleton and aStarOn) or (hallwayskel and highwayFinished >= 1 and aStarOn)){
    cout << "Updating skeleton planner" << endl;
    PathPlanner *skeleton_planner;
    for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
      if(skeleton and (*it)->getName() == "skeleton"){
        skeleton_planner = *it;
      }
      else if(hallwayskel and (*it)->getName() == "hallwayskel"){
        skeleton_planner = *it;
      }
    }
    if(skeleton){
      skeleton_planner->resetGraph();
    }
    else if(hallwayskel){
      skeleton_planner->resetOrigGraph();
    }
    // cout << "Planner reset" << endl;
    vector<FORRRegion> regions = beliefs->getSpatialModel()->getRegionList()->getRegions();
    int index_val = 0;
    for(int i = 0 ; i < regions.size(); i++){
      int x = (int)(regions[i].getCenter().get_x()*100);
      int y = (int)(regions[i].getCenter().get_y()*100);
      // cout << "Region " << regions[i].getCenter().get_x() << " " << regions[i].getCenter().get_y() << " " << x << " " << y << endl;
      vector<FORRExit> exits = regions[i].getMinExits();
      // cout << "Exits " << exits.size() << endl;
      if(exits.size() > 0){
        if(skeleton){
          bool success = skeleton_planner->getGraph()->addNode(x, y, regions[i].getRadius(), index_val);
          if(success){
            index_val++;
          }
        }
        else if(hallwayskel){
          bool success = skeleton_planner->getOrigGraph()->addNode(x, y, regions[i].getRadius(), index_val);
          if(success){
            index_val++;
          }
        }
      }
    }
    for(int i = 0 ; i < regions.size(); i++){
      if(skeleton){
        int region_id = skeleton_planner->getGraph()->getNodeID((int)(regions[i].getCenter().get_x()*100), (int)(regions[i].getCenter().get_y()*100));
        if(region_id != -1){
          vector<FORRExit> exits = regions[i].getMinExits();
          for(int j = 0; j < exits.size() ; j++){
            int index_val = skeleton_planner->getGraph()->getNodeID((int)(regions[exits[j].getExitRegion()].getCenter().get_x()*100), (int)(regions[exits[j].getExitRegion()].getCenter().get_y()*100));
            if(index_val != -1){
              skeleton_planner->getGraph()->addEdge(region_id, index_val, exits[j].getExitDistance()*100, exits[j].getConnectionPoints());
            }
          }
        }
      }
      else if(hallwayskel){
        int region_id = skeleton_planner->getOrigGraph()->getNodeID((int)(regions[i].getCenter().get_x()*100), (int)(regions[i].getCenter().get_y()*100));
        if(region_id != -1){
          vector<FORRExit> exits = regions[i].getMinExits();
          for(int j = 0; j < exits.size() ; j++){
            int index_val = skeleton_planner->getOrigGraph()->getNodeID((int)(regions[exits[j].getExitRegion()].getCenter().get_x()*100), (int)(regions[exits[j].getExitRegion()].getCenter().get_y()*100));
            if(index_val != -1){
              skeleton_planner->getOrigGraph()->addEdge(region_id, index_val, exits[j].getExitDistance()*100, exits[j].getConnectionPoints());
            }
          }
        }
      }
    }
    if(skeleton){
      cout << "Finished updating skeleton planner" << endl;
      // skeleton_planner->getGraph()->printGraph();
      // cout << "Connected Graph: " << skeleton_planner->getGraph()->isConnected() << endl;
    }
    else if(hallwayskel){
      cout << "Finished updating skeleton graph for passage planner" << endl;
      // skeleton_planner->getOrigGraph()->printGraph();
      beliefs->getSpatialModel()->getRegionList()->setRegionPassageValues(beliefs->getAgentState()->getPassageGrid());
      // cout << "Connected Graph: " << skeleton_planner->getOrigGraph()->isConnected() << endl;
    }
  }
  if(hallwayskel and highwayFinished == 1 and aStarOn){
    PathPlanner *hwskeleton_planner;
    for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
      if((*it)->getName() == "hallwayskel"){
        hwskeleton_planner = *it;
      }
    }
    hwskeleton_planner->resetGraph();
    vector< vector<int> > highway_grid = highwayExploration->getHighwayGrid();
    // vector< vector< vector< pair<int, int> > > > highway_grid_connections = highwayExploration->getHighwayGridConnections();
    // cout << "Highway grid" << endl;
    // for(int i = 0; i < highway_grid.size(); i++){
    //   for(int j = 0; j < highway_grid[0].size(); j++){
    //     cout << highway_grid[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector< vector<int> > decisions_grid;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j ++){
        col.push_back(0);
      }
      decisions_grid.push_back(col);
    }
    Task* completedTask = agentState->getCurrentTask();
    vector<Position> *pos_hist = completedTask->getPositionHistory();
    vector< vector<CartesianPoint> > *laser_hist = completedTask->getLaserHistory();
    vector< vector<CartesianPoint> > all_trace = beliefs->getAgentState()->getAllTrace();
    vector< vector < vector<CartesianPoint> > > all_laser_trace = beliefs->getAgentState()->getAllLaserTrace();
    vector<CartesianPoint> trace;
    for(int i = 0 ; i < pos_hist->size() ; i++){
      trace.push_back(CartesianPoint((*pos_hist)[i].getX(),(*pos_hist)[i].getY()));
    }
    all_trace.push_back(trace);
    vector < vector<CartesianPoint> > laser_trace;
    for(int i = 0 ; i < laser_hist->size() ; i++){
      laser_trace.push_back((*laser_hist)[i]);
    }
    all_laser_trace.push_back(laser_trace);
    vector<CartesianPoint> stepped_history;
    vector < vector<CartesianPoint> > stepped_laser_history;
    for(int k = 0; k < all_trace.size() ; k++){
      vector<CartesianPoint> history = all_trace[k];
      vector < vector<CartesianPoint> > laser_history = all_laser_trace[k];
      for(int j = 0; j < history.size(); j++){
        stepped_history.push_back(history[j]);
        stepped_laser_history.push_back(laser_history[j]);
      }
    }
    // cout << "stepped_history " << stepped_history.size() << " stepped_laser_history " << stepped_laser_history.size() << endl;
    double step_length = 1;
    for(int k = 0; k < stepped_history.size()-1 ; k++){
      double start_x = stepped_history[k].get_x();
      if(int(start_x) < 0)
        start_x = 0;
      if(int(start_x) >= decisions_grid.size())
        start_x = decisions_grid.size()-1;
      double start_y = stepped_history[k].get_y();
      if(int(start_y) < 0)
        start_y = 0;
      if(int(start_y) >= decisions_grid[0].size())
        start_y = decisions_grid[0].size()-1;
      double end_x = stepped_history[k+1].get_x();
      if(int(end_x) < 0)
        end_x = 0;
      if(int(end_x) >= decisions_grid.size())
        end_x = decisions_grid.size()-1;
      double end_y = stepped_history[k+1].get_y();
      if(int(end_y) < 0)
        end_y = 0;
      if(int(end_y) >= decisions_grid[0].size())
        end_y = decisions_grid[0].size()-1;
      decisions_grid[int(start_x)][int(start_y)] = 1;
      decisions_grid[int(end_x)][int(end_y)] = 1;
      // decisions_grid[floor(start_y)][floor(start_x)] = 1;
      // decisions_grid[floor(end_y)][floor(end_x)] = 1;
      // decisions_grid[ceil(start_y)][ceil(start_x)] = 1;
      // decisions_grid[ceil(end_y)][ceil(end_x)] = 1;
      // decisions_grid[floor(start_y)][ceil(start_x)] = 1;
      // decisions_grid[floor(end_y)][ceil(end_x)] = 1;
      // decisions_grid[ceil(start_y)][floor(start_x)] = 1;
      // decisions_grid[ceil(end_y)][floor(end_x)] = 1;
      double length = sqrt((start_x - end_x) * (start_x - end_x) + (start_y - end_y) * (start_y - end_y));
      // cout << k << " " << start_x << " " << start_y << " " << end_x << " " << end_y << " length " << length << endl;
      if(length >= step_length){
        double step_size = step_length / length;
        double tx, ty;
        // cout << "step_size " << step_size << endl;
        for(double j = 0; j <= 1; j += step_size){
          tx = (end_x * j) + (start_x * (1 - j));
          ty = (end_y * j) + (start_y * (1 - j));
          if(int(tx) >= 0 and int(ty) >= 0 and int(tx) < decisions_grid.size() and int(ty) < decisions_grid[0].size()){
            // cout << tx << " " << ty << endl;
            decisions_grid[int(tx)][int(ty)] = 1;
          }
          // decisions_grid[floor(ty)][floor(tx)] = 1
          // decisions_grid[ceil(ty)][ceil(tx)] = 1
          // decisions_grid[floor(ty)][ceil(tx)] = 1
          // decisions_grid[ceil(ty)][floor(tx)] = 1
        }
      }
    }
    // cout << "After decisions_grid" << endl;
    // for(int i = 0; i < decisions_grid.size(); i++){
    //   for(int j = 0; j < decisions_grid[0].size(); j++){
    //     cout << decisions_grid[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector< vector<int> > lasers_grid;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j ++){
        col.push_back(0);
      }
      lasers_grid.push_back(col);
    }
    for(int k = 0; k < stepped_laser_history.size(); k++){
      double start_x = stepped_history[k].get_x();
      if(int(start_x) < 0)
        start_x = 0;
      if(int(start_x) >= decisions_grid.size())
        start_x = decisions_grid.size()-1;
      double start_y = stepped_history[k].get_y();
      if(int(start_y) < 0)
        start_y = 0;
      if(int(start_y) >= decisions_grid[0].size())
        start_y = decisions_grid[0].size()-1;
      for(int j = 0; j < stepped_laser_history[k].size(); j++){
        double end_x = stepped_laser_history[k][j].get_x();
        if(int(end_x) < 0)
          end_x = 0;
        if(int(end_x) >= decisions_grid.size())
          end_x = decisions_grid.size()-1;
        double end_y = stepped_laser_history[k][j].get_y();
        if(int(end_y) < 0)
          end_y = 0;
        if(int(end_y) >= decisions_grid[0].size())
          end_y = decisions_grid[0].size()-1;
        if(decisions_grid[int(start_x)][int(start_y)] == 1 and decisions_grid[int(end_x)][int(end_y)] == 1 and ((int(start_y) == int(end_y)) or (int(start_x) == int(end_x)))){
          double length = sqrt((start_x - end_x) * (start_x - end_x) + (start_y - end_y) * (start_y - end_y));
          // cout << k << " " << start_x << " " << start_y << " " << end_x << " " << end_y << " length " << length << endl;
          if(length >= step_length){
            double step_size = step_length / length;
            double tx, ty;
            // cout << "step_size " << step_size << endl;
            for(double j = 0; j <= 1; j += step_size){
              tx = (end_x * j) + (start_x * (1 - j));
              ty = (end_y * j) + (start_y * (1 - j));
              if(int(tx) >= 0 and int(ty) >= 0 and int(tx) < decisions_grid.size() and int(ty) < decisions_grid[0].size()){
                // cout << tx << " " << ty << endl;
                lasers_grid[int(tx)][int(ty)] = 1;
              }
              // decisions_grid[floor(ty)][floor(tx)] = 1
              // decisions_grid[ceil(ty)][ceil(tx)] = 1
              // decisions_grid[floor(ty)][ceil(tx)] = 1
              // decisions_grid[ceil(ty)][floor(tx)] = 1
            }
          }
        }
      }
    }
    // cout << "After lasers_grid" << endl;
    // for(int i = 0; i < lasers_grid.size(); i++){
    //   for(int j = 0; j < lasers_grid[0].size(); j++){
    //     cout << lasers_grid[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    // cout << "After decisions_grid + lasers_grid" << endl;
    for(int i = 0; i < decisions_grid.size(); i++){
      for(int j = 0; j < decisions_grid[0].size(); j++){
        if(lasers_grid[i][j] > 0 and decisions_grid[i][j] == 0){
          decisions_grid[i][j] = 1;
        }
        // cout << decisions_grid[i][j] << " ";
      }
      // cout << endl;
    }
    vector< vector<int> > passage_grid;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j ++){
        if(highway_grid[i][j] < 0){
          vector<int> values;
          if(i > 0){
            if(highway_grid[i-1][j] >= 0)
              values.push_back(highway_grid[i-1][j]);
          }
          if(j > 0){
            if(highway_grid[i][j-1] >= 0)
              values.push_back(highway_grid[i][j-1]);
          }
          if(i < highway_grid.size()-1){
            if(highway_grid[i+1][j] >= 0)
              values.push_back(highway_grid[i+1][j]);
          }
          if(j < highway_grid[0].size()-1){
            if(highway_grid[i][j+1] >= 0)
              values.push_back(highway_grid[i][j+1]);
          }
          if(values.size() >= 3){
            col.push_back(1);
          }
          else if(decisions_grid[i][j] > 0){
            col.push_back(1);
          }
          else{
            col.push_back(-1);
          }
        }
        else{
          col.push_back(highway_grid[i][j]);
        }
      }
      passage_grid.push_back(col);
    }
    // cout << "After passage_grid" << endl;
    // for(int i = 0; i < passage_grid.size(); i++){
    //   for(int j = 0; j < passage_grid[0].size(); j++){
    //     cout << passage_grid[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector < vector < pair<int, int> > > horizontals;
    vector < vector <int> > horizontal_lengths;
    for(int i = 0; i < passage_grid.size(); i++){
      vector < pair<int, int> > row;
      vector <int> row_lengths;
      int start = -1;
      int stop = -1;
      for(int j = 0; j < passage_grid[i].size(); j++){
        if(passage_grid[i][j] >= 0 and start == -1){
          start = j;
          stop = j;
        }
        else if(passage_grid[i][j] >= 0 and start > -1){
          stop = j;
        }
        else if(passage_grid[i][j] < 0 and start > -1 and stop > -1){
          // cout << "row " << i << " start " << start << " stop " << stop << " length " << stop-start+1 << endl;
          row.push_back(make_pair(start, stop));
          row_lengths.push_back(stop-start+1);
          start = -1;
          stop = -1;
        }
      }
      horizontals.push_back(row);
      horizontal_lengths.push_back(row_lengths);
    }
    vector< vector<int> > horizontal_passages;
    vector< vector<int> > horizontal_passages_filled;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(-1);
      }
      horizontal_passages.push_back(col);
      horizontal_passages_filled.push_back(col);
    }
    for(int i = 0; i < horizontals.size(); i++){
      for(int j = 0; j < horizontals[i].size(); j++){
        if(horizontal_lengths[i][j] >= 10){
          for(int k = horizontals[i][j].first; k <= horizontals[i][j].second; k++){
            horizontal_passages[i][k] = 1;
            horizontal_passages_filled[i][k] = 1;
          }
        }
      }
    }
    // cout << "After horizontal_passages" << endl;
    // for(int i = 0; i < horizontal_passages.size(); i++){
    //   for(int j = 0; j < horizontal_passages[0].size(); j++){
    //     cout << horizontal_passages[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    for(int i = 0; i < passage_grid.size(); i++){
      for(int j = 0; j < passage_grid[i].size(); j++){
        if(passage_grid[i][j] >= 0 and horizontal_passages[i][j] == -1){
          if(i > 0 and i < passage_grid.size()-1){
            if(horizontal_passages[i-1][j] >= 0 or horizontal_passages[i+1][j] >= 0){
              horizontal_passages_filled[i][j] = 1;
            }
          }
          else if(i == 0){
            if(horizontal_passages[i+1][j] >= 0){
              horizontal_passages_filled[i][j] = 1;
            }
          }
          else if(i == passage_grid.size()-1){
            if(horizontal_passages[i-1][j] >= 0){
              horizontal_passages_filled[i][j] = 1;
            }
          }
        }
      }
    }
    // cout << "After horizontal_passages_filled" << endl;
    // for(int i = 0; i < horizontal_passages_filled.size(); i++){
    //   for(int j = 0; j < horizontal_passages_filled[0].size(); j++){
    //     cout << horizontal_passages_filled[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector<int> dx;
    dx.push_back(1);
    dx.push_back(0);
    dx.push_back(-1);
    dx.push_back(0);
    dx.push_back(1);
    dx.push_back(1);
    dx.push_back(-1);
    dx.push_back(-1);
    vector<int> dy;
    dy.push_back(0);
    dy.push_back(1);
    dy.push_back(0);
    dy.push_back(-1);
    dy.push_back(1);
    dy.push_back(-1);
    dy.push_back(1);
    dy.push_back(-1);

    // const int dx[] = {+1, 0, -1, 0, +1, +1, -1, -1};
    // const int dy[] = {0, +1, 0, -1, +1, -1, +1, -1};
    int row_count = horizontal_passages_filled.size();
    int col_count = horizontal_passages_filled[0].size();
    vector< vector<int> > final_horizontal;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(0);
      }
      final_horizontal.push_back(col);
    }
    int horizontal_component = 0;
    for(int i = 0; i < row_count; ++i){
      for(int j = 0; j < col_count; ++j){
        if(final_horizontal[i][j] == 0 && horizontal_passages_filled[i][j] >= 0){
          agentState->dfs(i, j, ++horizontal_component, dx, dy, row_count, col_count, &final_horizontal, &horizontal_passages_filled);
        }
      }
    }
    // cout << "final horizontal_component " << horizontal_component << endl;
    // cout << "After final_horizontal" << endl;
    // for(int i = 0; i < final_horizontal.size(); i++){
    //   for(int j = 0; j < final_horizontal[0].size(); j++){
    //     cout << final_horizontal[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector < vector<int> > horizontal_ends;
    for(int i = 1; i <= horizontal_component; i++){
      // cout << "horizontal_component " << i << endl;
      int right = -1;
      int left = final_horizontal[0].size();
      // cout << right << " " << left << endl;
      for(int j = 0; j < final_horizontal.size(); j++){
        for(int k = 0; k < final_horizontal[j].size(); k++){
          if(final_horizontal[j][k] == i){
            if(k > right){
              right = k;
            }
            if(k < left){
              left = k;
            }
            // cout << right << " " << left << endl;
          }
        }
      }
      for(int j = 0; j < final_horizontal.size(); j++){
        if(final_horizontal[j][right] == i){
          vector<int> point;
          point.push_back(j);
          point.push_back(right);
          // cout << j << " " << right << endl;
          horizontal_ends.push_back(point);
        }
        if(final_horizontal[j][left] == i){
          vector<int> point;
          point.push_back(j);
          point.push_back(left);
          // cout << j << " " << left << endl;
          horizontal_ends.push_back(point);
        }
      }
    }
    // cout << "After horizontal_ends " << horizontal_ends.size() << endl;

    vector < vector < pair<int, int> > > verticals;
    vector < vector <int> > vertical_lengths;
    for(int j = 0; j < passage_grid[0].size(); j++){
      vector < pair<int, int> > col;
      vector <int> col_lengths;
      int start = -1;
      int stop = -1;
      for(int i = 0; i < passage_grid.size(); i++){
        if(passage_grid[i][j] >= 0 and start == -1){
          start = i;
          stop = i;
        }
        else if(passage_grid[i][j] >= 0 and start > -1){
          stop = i;
        }
        else if(passage_grid[i][j] < 0 and start > -1 and stop > -1){
          // cout << "col " << i << " start " << start << " stop " << stop << " length " << stop-start+1 << endl;
          col.push_back(make_pair(start, stop));
          col_lengths.push_back(stop-start+1);
          start = -1;
          stop = -1;
        }
      }
      verticals.push_back(col);
      vertical_lengths.push_back(col_lengths);
    }
    vector< vector<int> > vertical_passages;
    vector< vector<int> > vertical_passages_filled;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(-1);
      }
      vertical_passages.push_back(col);
      vertical_passages_filled.push_back(col);
    }

    for(int i = 0; i < verticals.size(); i++){
      for(int j = 0; j < verticals[i].size(); j++){
        if(vertical_lengths[i][j] >= 10){
          for(int k = verticals[i][j].first; k <= verticals[i][j].second; k++){
            vertical_passages[k][i] = 1;
            vertical_passages_filled[k][i] = 1;
          }
        }
      }
    }
    // cout << "After vertical_passages" << endl;
    // for(int i = 0; i < vertical_passages.size(); i++){
    //   for(int j = 0; j < vertical_passages[0].size(); j++){
    //     cout << vertical_passages[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    for(int i = 0; i < passage_grid.size(); i++){
      for(int j = 0; j < passage_grid[i].size(); j++){
        if(passage_grid[i][j] >= 0 and vertical_passages[i][j] == -1){
          if(j > 0 and j < passage_grid[i].size()-1){
            if(vertical_passages[i][j-1] >= 0 or vertical_passages[i][j+1] >= 0){
              vertical_passages_filled[i][j] = 1;
            }
          }
          else if(j == 0){
            if(vertical_passages[i][j+1] >= 0){
              vertical_passages_filled[i][j] = 1;
            }
          }
          else if(j == passage_grid[i].size()-1){
            if(vertical_passages[i][j-1] >= 0){
              vertical_passages_filled[i][j] = 1;
            }
          }
        }
      }
    }
    // cout << "After vertical_passages_filled" << endl;
    // for(int i = 0; i < vertical_passages_filled.size(); i++){
    //   for(int j = 0; j < vertical_passages_filled[0].size(); j++){
    //     cout << vertical_passages_filled[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    row_count = vertical_passages_filled.size();
    col_count = vertical_passages_filled[0].size();
    vector< vector<int> > final_vertical;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(0);
      }
      final_vertical.push_back(col);
    }
    int vertical_component = 0;
    for(int i = 0; i < row_count; ++i){
      for(int j = 0; j < col_count; ++j){
        if(final_vertical[i][j] == 0 && vertical_passages_filled[i][j] >= 0){
          agentState->dfs(i, j, ++vertical_component, dx, dy, row_count, col_count, &final_vertical, &vertical_passages_filled);
        }
      }
    }
    // cout << "final vertical_component " << vertical_component << endl;
    // cout << "After final_vertical" << endl;
    // for(int i = 0; i < final_vertical.size(); i++){
    //   for(int j = 0; j < final_vertical[0].size(); j++){
    //     cout << final_vertical[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector < vector<int> > vertical_ends;
    for(int i = 1; i <= vertical_component; i++){
      // cout << "vertical_component " << i << endl;
      int bottom = -1;
      int top = final_vertical[0].size();
      // cout << bottom << " " << top << endl;
      for(int j = 0; j < final_vertical.size(); j++){
        for(int k = 0; k < final_vertical[j].size(); k++){
          if(final_vertical[j][k] == i){
            if(j > bottom){
              bottom = j;
            }
            if(j < top){
              top = j;
            }
            // cout << bottom << " " << top << endl;
          }
        }
      }
      for(int k = 0; k < final_vertical[0].size(); k++){
        if(final_vertical[bottom][k] == i){
          vector<int> point;
          point.push_back(bottom);
          point.push_back(k);
          // cout << bottom << " " << k << endl;
          vertical_ends.push_back(point);
        }
        if(final_vertical[top][k] == i){
          vector<int> point;
          point.push_back(top);
          point.push_back(k);
          // cout << top << " " << k << endl;
          vertical_ends.push_back(point);
        }
      }
    }
    // cout << "After vertical_ends " << vertical_ends.size() << endl;

    vector < vector<int> > final_combined;
    for(int i = 0; i < final_horizontal.size(); i++){
      vector<int> row;
      for(int j = 0; j < final_horizontal[i].size(); j++){
        if(final_horizontal[i][j] > 0 and final_vertical[i][j] > 0){
          row.push_back(1);
        }
        else{
          row.push_back(-1);
        }
      }
      final_combined.push_back(row);
    }
    // cout << "After final_combined" << endl;
    // for(int i = 0; i < final_combined.size(); i++){
    //   for(int j = 0; j < final_combined[0].size(); j++){
    //     cout << final_combined[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    for(int i = 0; i < horizontal_ends.size(); i++){
      final_combined[horizontal_ends[i][0]][horizontal_ends[i][1]] = 1;
    }
    for(int i = 0; i < vertical_ends.size(); i++){
      final_combined[vertical_ends[i][0]][vertical_ends[i][1]] = 1;
    }
    // cout << "After ends" << endl;
    // for(int i = 0; i < final_combined.size(); i++){
    //   for(int j = 0; j < final_combined[0].size(); j++){
    //     cout << final_combined[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    dx.clear();
    dx.push_back(1);
    dx.push_back(0);
    dx.push_back(-1);
    dx.push_back(0);
    dy.clear();
    dy.push_back(0);
    dy.push_back(1);
    dy.push_back(0);
    dy.push_back(-1);
    row_count = final_combined.size();
    col_count = final_combined[0].size();
    vector< vector<int> > intersections;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(0);
      }
      intersections.push_back(col);
    }
    int intersection_component = 0;
    for(int i = 0; i < row_count; ++i){
      for(int j = 0; j < col_count; ++j){
        if(intersections[i][j] == 0 && final_combined[i][j] >= 0){
          agentState->dfs(i, j, ++intersection_component, dx, dy, row_count, col_count, &intersections, &final_combined);
        }
      }
    }
    // cout << "final intersection_component " << intersection_component << endl;
    // cout << "After intersections" << endl;
    // for(int i = 0; i < intersections.size(); i++){
    //   for(int j = 0; j < intersections[0].size(); j++){
    //     cout << intersections[i][j] << " ";
    //   }
    //   cout << endl;
    // }

    vector < vector<int> > passages_without_intersections;
    for(int i = 0; i < final_horizontal.size(); i++){
      vector<int> row;
      for(int j = 0; j < final_horizontal[i].size(); j++){
        if(intersections[i][j] == 0 and (final_horizontal[i][j] > 0 or final_vertical[i][j] > 0)){
          row.push_back(1);
        }
        else{
          row.push_back(-1);
        }
      }
      passages_without_intersections.push_back(row);
    }
    // cout << "After passages_without_intersections" << endl;
    // for(int i = 0; i < passages_without_intersections.size(); i++){
    //   for(int j = 0; j < passages_without_intersections[0].size(); j++){
    //     cout << passages_without_intersections[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    row_count = passages_without_intersections.size();
    col_count = passages_without_intersections[0].size();
    vector< vector<int> > pass_wo_int;
    for(int i = 0; i < highway_grid.size(); i++){
      vector<int> col;
      for(int j = 0; j < highway_grid[i].size(); j++){
        col.push_back(0);
      }
      pass_wo_int.push_back(col);
    }
    int passage_component = 0;
    for(int i = 0; i < row_count; ++i){
      for(int j = 0; j < col_count; ++j){
        if(pass_wo_int[i][j] == 0 && passages_without_intersections[i][j] >= 0){
          agentState->dfs(i, j, ++passage_component, dx, dy, row_count, col_count, &pass_wo_int, &passages_without_intersections);
        }
      }
    }
    // cout << "final passage_component " << passage_component << endl;
    // cout << "After pass_wo_int" << endl;
    // for(int i = 0; i < pass_wo_int.size(); i++){
    //   for(int j = 0; j < pass_wo_int[0].size(); j++){
    //     cout << pass_wo_int[i][j] << " ";
    //   }
    //   cout << endl;
    // }

    int new_ind = intersection_component;
    for(int i = 0; i < pass_wo_int.size(); i++){
      for(int j = 0; j < pass_wo_int[i].size(); j++){
        if(pass_wo_int[i][j] > 0 and intersections[i][j] == 0){
          intersections[i][j] = pass_wo_int[i][j] + new_ind;
        }
      }
    }
    // cout << "After intersections + pass_wo_int" << endl;
    // for(int i = 0; i < intersections.size(); i++){
    //   for(int j = 0; j < intersections[0].size(); j++){
    //     cout << intersections[i][j] << " ";
    //   }
    //   cout << endl;
    // }
    vector< vector<int> > edges;
    for(int i = 0; i < intersections.size(); i++){
      int start = -1;
      int stop = -1;
      for(int j = 0; j < intersections[0].size(); j++){
        if(intersections[i][j] > 0 and start == -1){
          start = intersections[i][j];
        }
        else if((intersections[i][j] > 0 or intersections[i][j] == -1) and start > -1 and (stop == -1 or stop == start)){
          stop = intersections[i][j];
        }
        else if(intersections[i][j] == 0){
          start = -1;
          stop = -1;
        }
        if(start > -1 and stop > -1){
          vector<int> current_pair;
          if(start < stop){
            current_pair.push_back(start);
            current_pair.push_back(stop);
          }
          else{
            current_pair.push_back(stop);
            current_pair.push_back(start);
          }
          if(start != stop and find(edges.begin(), edges.end(), current_pair) == edges.end()){
            edges.push_back(current_pair);
          }
          start = stop;
          stop = -1;
        }
      }
    }
    for(int j = 0; j < intersections[0].size(); j++){
      int start = -1;
      int stop = -1;
      for(int i = 0; i < intersections.size(); i++){
        if(intersections[i][j] > 0 and start == -1){
          start = intersections[i][j];
        }
        else if((intersections[i][j] > 0 or intersections[i][j] == -1) and start > -1 and (stop == -1 or stop == start)){
          stop = intersections[i][j];
        }
        else if(intersections[i][j] == 0){
          start = -1;
          stop = -1;
        }
        if(start > -1 and stop > -1){
          vector<int> current_pair;
          if(start < stop){
            current_pair.push_back(start);
            current_pair.push_back(stop);
          }
          else{
            current_pair.push_back(stop);
            current_pair.push_back(start);
          }
          if(start != stop and find(edges.begin(), edges.end(), current_pair) == edges.end()){
            edges.push_back(current_pair);
          }
          start = stop;
          stop = -1;
        }
      }
    }
    // cout << "After edges" << endl;
    // for(int i = 0; i < edges.size(); i++){
    //   cout << "edge " << i << " connecting " << edges[i][0] << " " << edges[i][1] << endl;
    // }
    vector< vector<int> > graph;
    map<int, vector< vector<int> > > graph_nodes;
    map<int, vector< vector<int> > > graph_edges_map;
    vector<int> graph_edges;
    // cout << "graph_nodes" << endl;
    for(int i = 1; i < new_ind+1; i++){
      graph_nodes.insert(make_pair(i, vector< vector<int> >()));
      // cout << i << endl;
    }
    // cout << "graph_edges" << endl;
    for(int i = new_ind+1; i < new_ind+passage_component+1; i++){
      graph_edges.push_back(i);
      graph_edges_map.insert(make_pair(i, vector< vector<int> >()));
      // cout << i << endl;
    }
    for(int i = 0; i < graph_edges.size(); i++){
      vector< vector<int> > matches;
      for(int j = 0; j < edges.size(); j++){
        if(edges[j][1] == graph_edges[i]){
          matches.push_back(edges[j]);
        }
      }
      for(int j = 0; j < matches.size()-1; j++){
        for(int k = j+1; k < matches.size(); k++){
          vector<int> seq;
          seq.push_back(matches[j][0]);
          seq.push_back(matches[j][1]);
          seq.push_back(matches[k][0]);
          graph.push_back(seq);
        }
      }
    }
    // cout << "after graph" << endl;
    // for(int i = 0; i < graph.size(); i++){
    //   cout << graph[i][0] << " " << graph[i][1] << " " << graph[i][2] << endl;
    // }

    for(int i = 0; i < intersections.size(); i++){
      for(int j = 0; j < intersections[0].size(); j++){
        if(intersections[i][j] < new_ind+1 and intersections[i][j] > 0){
          vector<int> current_grid;
          current_grid.push_back(i);
          current_grid.push_back(j);
          graph_nodes[intersections[i][j]].push_back(current_grid);
          // cout << "node " << intersections[i][j] << " point " << i << " " << j << endl;
        }
        else if(intersections[i][j] >= new_ind+1){
          vector<int> current_grid;
          current_grid.push_back(i);
          current_grid.push_back(j);
          graph_edges_map[intersections[i][j]].push_back(current_grid);
          // cout << "edge " << intersections[i][j] << " point " << i << " " << j << endl;
        }
      }
    }
    // cout << "graph_nodes " << graph_nodes.size() << " graph_edges_map " << graph_edges_map.size() << endl;
    vector< vector<int> > average_passage;
    int index_val = 0;
    for(int i = 1; i < new_ind+1; i++){
      vector< vector<int> > points = graph_nodes[i];
      double x = 0, y = 0;
      for(int j = 0; j < points.size(); j++){
        x += points[j][0]*2;
        y += points[j][1]*2;
        x += (points[j][0]+1)*2;
        y += (points[j][1]+1)*2;
      }
      x = x / (points.size()*4);
      y = y / (points.size()*4);
      // cout << "node " << i << " index_val " << index_val << " x " << x << " y " << y << " node_x " << (int)(x*100) << " node_y " << (int)(y*100) << endl;
      vector<int> avg_psg;
      avg_psg.push_back((int)(x*100));
      avg_psg.push_back((int)(y*100));
      average_passage.push_back(avg_psg);
      bool success = hwskeleton_planner->getGraph()->addNode((int)(x*100), (int)(y*100), 0, index_val);
      if(success){
        index_val++;
      }
    }
    // cout << "finished creating nodes" << endl;
    for(int i = 0; i < graph.size(); i++){
      int node_a_id = hwskeleton_planner->getGraph()->getNodeID(average_passage[graph[i][0]-1][0], average_passage[graph[i][0]-1][1]);
      int node_b_id = hwskeleton_planner->getGraph()->getNodeID(average_passage[graph[i][2]-1][0], average_passage[graph[i][2]-1][1]);
      // cout << "graph " << graph[i][0] << " " << graph[i][1] << " " << graph[i][2] << " node_a_id " << node_a_id << " node_b_id " << node_b_id << endl;
      if(node_a_id != -1 and node_b_id != -1){
        double distance_ab = sqrt((average_passage[graph[i][0]-1][0] - average_passage[graph[i][2]-1][0])*(average_passage[graph[i][0]-1][0] - average_passage[graph[i][2]-1][0]) + (average_passage[graph[i][0]-1][1] - average_passage[graph[i][2]-1][1])*(average_passage[graph[i][0]-1][1] - average_passage[graph[i][2]-1][1]));
        vector<CartesianPoint> path;
        path.push_back(CartesianPoint(graph[i][0], -1));
        path.push_back(CartesianPoint(graph[i][1], -1));
        path.push_back(CartesianPoint(graph[i][2], -1));
        // cout << "distance_ab " << distance_ab << " path " << path.size() << endl;
        hwskeleton_planner->getGraph()->addEdge(node_a_id, node_b_id, distance_ab, path);
      }
    }
    // cout << "finished creating edges" << endl;
    // hwskeleton_planner->getGraph()->printGraph();
    // cout << "Connected Graph: " << hwskeleton_planner->getGraph()->isConnected() << endl;
    for(int i = new_ind+1; i < new_ind+passage_component+1; i++){
      vector< vector<int> > points = graph_edges_map[i];
      double x = 0, y = 0;
      for(int j = 0; j < points.size(); j++){
        x += points[j][0]*2;
        y += points[j][1]*2;
        x += (points[j][0]+1)*2;
        y += (points[j][1]+1)*2;
      }
      x = x / (points.size()*4);
      y = y / (points.size()*4);
      // cout << "edge " << i << " index_val " << index_val << " x " << x << " y " << y << " edge_x " << (int)(x*100) << " edge_y " << (int)(y*100) << endl;
      vector<int> avg_psg;
      avg_psg.push_back((int)(x*100));
      avg_psg.push_back((int)(y*100));
      average_passage.push_back(avg_psg);
      index_val++;
    }
    // cout << "graph_nodes " << graph_nodes.size() << " graph_edges_map " << graph_edges_map.size() << endl;
    // for(int k = 0; k < graph.size(); k++){
    //   cout << graph[k][0] << " " << graph[k][1] << " " << graph[k][2] << endl;
    //   for(int i = 0; i < intersections.size(); i++){
    //     for(int j = 0; j < intersections[0].size(); j++){
    //       if(intersections[i][j] == graph[k][0] or intersections[i][j] == graph[k][2]){
    //         cout << intersections[i][j] << " " << i << " " << j << endl;
    //         vector<int> current_grid;
    //         current_grid.push_back(i);
    //         current_grid.push_back(j);
    //         graph_edges_map[graph[k][1]].push_back(current_grid);
    //       }
    //     }
    //   }
    //   // vector< vector<int> > intersection1_points = graph_nodes[graph[i][0]];
    //   // for(int j = 0; j < intersection1_points.size(); j++){
    //   //   graph_edges_map[graph[i][1]].push_back(intersection1_points[j]);
    //   // }
    //   // vector< vector<int> > intersection2_points = graph_nodes[graph[i][1]];
    //   // for(int j = 0; j < intersection2_points.size(); j++){
    //   //   graph_edges_map[graph[i][1]].push_back(intersection2_points[j]);
    //   // }
    // }
    // cout << "graph_nodes " << graph_nodes.size() << " graph_edges_map " << graph_edges_map.size() << endl;
    agentState->setPassageValues(intersections, graph_nodes, graph_edges_map, graph, average_passage);
    beliefs->getSpatialModel()->getRegionList()->setRegionPassageValues(intersections);
    cout << "Finished updating planner planner" << endl;
  }

  gettimeofday(&cv,NULL);
  end_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
  computationTimeSec = (end_timecv-start_timecv);
  decisionStats->graphingComputationTime = computationTimeSec;
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
  // if(decision->type == FORWARD or decision->type == PAUSE){
  //   beliefs->getAgentState()->setRotateMode(true);
  // }
  // else{
  //   beliefs->getAgentState()->setRotateMode(false);
  // }
  return *decision;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Generate tier 1 decision
//
//
bool Controller::tierOneDecision(FORRAction *decision){
  //decision making tier1 advisor
  bool decisionMade = false;
  // ROS_INFO("Advisor circumnavigate will create subplan");
  // tier1->advisorCircumnavigate(decision);
  vector<Position> *positionHis = beliefs->getAgentState()->getCurrentTask()->getPositionHistory();
  if(positionHis->size() > 1){
    CartesianPoint current_position = CartesianPoint(positionHis->at(positionHis->size()-1).getX(), positionHis->at(positionHis->size()-1).getY());
    if(current_position.get_distance(beliefs->getAgentState()->getFarthestPoint()) <= 0.1){
      beliefs->getAgentState()->setGetOutTriggered(false);
    }
  }
  if(tier1->advisorVictory(decision)){ 
    ROS_INFO_STREAM("Advisor Victory has made a decision " << decision->type << " " << decision->parameter);
    // circumnavigator->addToStack(beliefs->getAgentState()->getCurrentPosition(), beliefs->getAgentState()->getCurrentLaserScan());
    decisionStats->decisionTier = 1;
    decisionMade = true;
  }
  else{
    ROS_INFO("Advisor AvoidObstacles will veto actions");
    tier1->advisorAvoidObstacles();
    ROS_INFO("Advisor NotOpposite will veto actions");
    tier1->advisorNotOpposite();
    if(tier1->advisorEnforcer(decision)){ 
      ROS_INFO_STREAM("Advisor Enforcer has made a decision " << decision->type << " " << decision->parameter);
      // circumnavigator->addToStack(beliefs->getAgentState()->getCurrentPosition(), beliefs->getAgentState()->getCurrentLaserScan());
      decisionStats->decisionTier = 1;
      decisionMade = true;
    }
    // if(circumnavigator->advisorCircumnavigate(decision)){
    //   ROS_INFO_STREAM("Advisor circumnavigate has made a decision " << decision->type << " " << decision->parameter);
    //   decisionStats->decisionTier = 2.5;
    //   decisionMade = true;
    // }
    // else
    if(outofhereOn){
      if(tier1->advisorGetOut(decision)){
        ROS_INFO_STREAM("Advisor get out has made a decision " << decision->type << " " << decision->parameter);
        decisionStats->decisionTier = 1;
        decisionMade = true;
      }
    }
    // else{
    //   // group of vetoing tier1 advisors which adds to the list of vetoed actions
    //   ROS_INFO("Advisor don't go back will veto actions");
    //   tier1->advisorDontGoBack();
    // }
    // if(situationsOn){
    //   ROS_INFO("Advisor situation will veto actions");
    //   tier1->advisorSituation();
    // }
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
// Generate tier 2 decision
//
//
void Controller::tierTwoDecision(Position current){
  ROS_DEBUG_STREAM("Tier 2 Decision");
  vector< list<int> > plans;
  vector<string> plannerNames;
  typedef vector< list<int> >::iterator vecIT;

  beliefs->getAgentState()->setCurrentTask(beliefs->getAgentState()->getNextTask());

  double computationTimeSec=0.0;
  timeval cv;
  double start_timecv;
  double end_timecv;
  gettimeofday(&cv,NULL);
  start_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
  bool planCreated = false;
  for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
    PathPlanner *planner = *it;
    planner->setPosHistory(beliefs->getAgentState()->getAllTrace());
    vector< vector<CartesianPoint> > trails_trace = beliefs->getSpatialModel()->getTrails()->getTrailsPoints();
    planner->setSpatialModel(beliefs->getSpatialModel()->getConveyors(),beliefs->getSpatialModel()->getRegionList()->getRegions(),beliefs->getSpatialModel()->getDoors()->getDoors(),trails_trace,beliefs->getSpatialModel()->getHallways()->getHallways());
    if(highwayFinished >= 1){
      planner->setPassageGrid(beliefs->getAgentState()->getPassageGrid(), beliefs->getAgentState()->getPassageGraphNodes(), beliefs->getAgentState()->getPassageGraph(), beliefs->getAgentState()->getAveragePassage());
      beliefs->getAgentState()->getCurrentTask()->setPassageValues(beliefs->getAgentState()->getPassageGrid(), beliefs->getAgentState()->getPassageGraphNodes(), beliefs->getAgentState()->getPassageGraphEdges(), beliefs->getAgentState()->getPassageGraph(), beliefs->getAgentState()->getAveragePassage());
    }
    //ROS_DEBUG_STREAM("Creating plans " << planner->getName());
    //gettimeofday(&cv,NULL);
    //start_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
    vector< list<int> > multPlans = beliefs->getAgentState()->getPlansWaypoints(current,planner,aStarOn);
    for (int i = 0; i < multPlans.size(); i++){
      plans.push_back(multPlans[i]);
      plannerNames.push_back(planner->getName());
      if(multPlans[i].size() > 0){
        planCreated = true;
      }
    }
    //gettimeofday(&cv,NULL);
    //end_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
    //computationTimeSec = (end_timecv-start_timecv);
    //ROS_DEBUG_STREAM("Planning time = " << computationTimeSec);
  }
  if(planCreated == true){
    vector< vector<double> > planCosts;
    typedef vector< vector<double> >::iterator costIT;

    for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
      PathPlanner *planner = *it;
      vector<double> planCost;
      // ROS_DEBUG_STREAM("Computing plan cost " << planner->getName());
      for (vecIT vt = plans.begin(); vt != plans.end(); vt++){
        double costOfPlan = planner->calcPathCost(*vt);
        planCost.push_back(costOfPlan);
        // ROS_DEBUG_STREAM("Cost = " << costOfPlan);
      }
      planCosts.push_back(planCost);
    }

    typedef vector<double>::iterator doubIT;
    vector< vector<double> > planCostsNormalized;
    for (costIT it = planCosts.begin(); it != planCosts.end(); it++){
      double max = *max_element(it->begin(), it->end());
      double min = *min_element(it->begin(), it->end());
      double norm_factor = (max - min)/10;
      vector<double> planCostNormalized;
      // ROS_DEBUG_STREAM("Computing normalized plan cost: Max = " << max << " Min = " << min << " Norm Factor = " << norm_factor);
      for (doubIT vt = it->begin(); vt != it->end(); vt++){
        if (max != min){
          planCostNormalized.push_back((*vt - min)/norm_factor);
          // ROS_DEBUG_STREAM("Original value = " << *vt << " Normalized = " << ((*vt - min)/norm_factor));
        }
        else{
          planCostNormalized.push_back(0);
          // ROS_DEBUG_STREAM("Original value = " << *vt << " Normalized = 0");
        }
      }
      planCostsNormalized.push_back(planCostNormalized);
    }
    //planCostsNormalized.pop_back();
    vector<double> totalCosts;
    for (int i = 0; i < plans.size(); i++){
      double cost=0;
      // ROS_DEBUG_STREAM("Computing total cost = " << cost);
      for (costIT it = planCostsNormalized.begin(); it != planCostsNormalized.end(); it++){
        cost += it->at(i);
        // ROS_DEBUG_STREAM("cost = " << cost);
      }
      // ROS_DEBUG_STREAM("Final cost = " << cost);
      totalCosts.push_back(cost);
    }
    double minCost=100000;
    // ROS_DEBUG_STREAM("Computing min cost");
    for (int i=0; i < totalCosts.size(); i++){
      // ROS_DEBUG_STREAM("Total cost = " << totalCosts[i]);
      if (totalCosts[i] < minCost){
        minCost = totalCosts[i];
      }
    }

    /*double minCombinedCost=1000;
    for (int i=18; i < totalCosts.size(); i++){
      ROS_DEBUG_STREAM("Total cost = " << totalCosts[i]);
      if (totalCosts[i] < minCombinedCost){
        minCombinedCost = totalCosts[i];
      }
    }*/
    // ROS_DEBUG_STREAM("Min cost = " << minCost);
    //ROS_DEBUG_STREAM("Min Combined cost = " << minCombinedCost);

    vector<string> bestPlanNames;
    vector<int> bestPlanInds;
    for (int i=0; i < totalCosts.size(); i++){
      if(totalCosts[i] == minCost){
        bestPlanNames.push_back(plannerNames[i]);
        bestPlanInds.push_back(i);
        // ROS_DEBUG_STREAM("Best plan " << plannerNames[i]);
      }
    }

    srand(time(NULL));
    int random_number = rand() % (bestPlanInds.size());
    ROS_DEBUG_STREAM("Number of best plans = " << bestPlanInds.size() << " random_number = " << random_number);
    ROS_DEBUG_STREAM("Selected Best plan " << bestPlanNames.at(random_number));
    decisionStats->chosenPlanner = bestPlanNames.at(random_number);
    for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
      PathPlanner *planner = *it;
      if(planner->getName() == bestPlanNames.at(random_number)){
        beliefs->getAgentState()->setCurrentWaypoints(current, beliefs->getAgentState()->getCurrentLaserEndpoints(), planner, aStarOn, plans.at(bestPlanInds.at(random_number)));
        break;
      }
    }
  }
  for (planner2It it = tier2Planners.begin(); it != tier2Planners.end(); it++){
    PathPlanner *planner = *it;
    planner->resetPath();
  }
  gettimeofday(&cv,NULL);
  end_timecv = cv.tv_sec + (cv.tv_usec/1000000.0);
  computationTimeSec = (end_timecv-start_timecv);
  decisionStats->planningComputationTime = computationTimeSec;
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
  // cout << "processing advisors::"<< endl;
  for (advisor3It it = tier3Advisors.begin(); it != tier3Advisors.end(); ++it){
    Tier3Advisor *advisor = *it; 
    // cout << advisor->get_name() << endl;
    // check if advisor should make a decision
    advisor->set_commenting();
    if(advisor->is_active() == false){
      //cout << advisor->get_name() << " is inactive " << endl;
      advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";
      continue;
    }
    if(advisor->is_commenting() == false){
      //cout << advisor->get_name() << " is not commenting " << endl;
      advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";
      continue;
    }

    advisorsList << advisor->get_name() << " " << advisor->get_weight() << " " << advisor->is_active() << " " << advisor->is_commenting() << ";";

    // cout << "Before commenting " << endl;
    comments = advisor->allAdvice();
    // cout << "after commenting " << endl;
    // aggregate all comments

    for(mapIt iterator = comments.begin(); iterator != comments.end(); iterator++){
      //cout << "comment : " << (iterator->first.type) << (iterator->first.parameter) << " " << (iterator->second) << endl;
      // If this is first advisor we need to initialize our final map
      float weight;
      //cout << "Agenda size :::::::::::::::::::::::::::::::::: " << beliefs->getAgenda().size() << endl;
      // cout << "<" << advisor->get_name() << "," << iterator->first.type << "," << iterator->first.parameter << "> : " << iterator->second << endl; 
      weight = advisor->get_weight();
      //cout << "Weight for this advisor : " << weight << endl;
      // if(advisor->get_name() == "Explorer" or advisor->get_name() == "ExplorerRotation" or advisor->get_name() == "LearnSpatialModel" or advisor->get_name() == "LearnSpatialModelRotation" or advisor->get_name() == "Curiosity" or advisor->get_name() == "CuriosityRotation"){
      //   weight = beliefs->getAgentState()->getAgenda().size()/5;
      // }

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
  double maxWeight;
  for(mapIt iterator = allComments.begin(); iterator != allComments.end(); iterator++){
    double action_weight = 1.0;
    if(situationsOn){
      action_weight = beliefs->getSpatialModel()->getSituations()->getWeightForAction(beliefs->getAgentState(), iterator->first);
    }
    // cout << "Values are : " << iterator->first.type << " " << iterator->first.parameter << " with value: " << iterator->second << " and weight: " << action_weight << endl;
    if(action_weight * iterator->second > maxAdviceStrength){
      maxAdviceStrength = action_weight * iterator->second;
      maxWeight = action_weight;
    }
  }
  // cout << "Max vote strength " << maxAdviceStrength << endl;
  
  for(mapIt iterator = allComments.begin(); iterator!=allComments.end(); iterator++){
    if(maxWeight * iterator->second == maxAdviceStrength)
      best_decisions.push_back(iterator->first);
  }
  
  // cout << "There are " << best_decisions.size() << " decisions that got the highest grade " << endl;
  if(best_decisions.size() == 0){
      (*decision) = FORRAction(PAUSE,0);
  }
  //for(unsigned i = 0; i < best_decisions.size(); ++i)
      //cout << "Action type: " << best_decisions.at(i).type << " parameter: " << best_decisions.at(i).parameter << endl;
    
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
