/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

using namespace ns3;

int main (int argc, char *argv[])
{
  RngSeedManager::SetSeed(time(NULL));
  // Set default parameters for cmd inputs
  std::string allocator = "uniformgrid";
  uint32_t numNodes = 100;
  uint32_t dimension = 10;
  int32_t radius = 3;
  double width = 5;
  uint32_t duration = 50;
  std::string minSpeed = "4.0";
  std::string maxSpeed = "6.0";
  std::string pause = "2.0";


  // Read cmd inputs
  CommandLine cmd (__FILE__);
  cmd.AddValue ("allocator", "The position allocator used", allocator);
  cmd.AddValue ("numNodes", "Number of nodes for the mobility model", numNodes);
  cmd.AddValue ("dimension", "X and Y dimension of the grid", dimension);
  cmd.AddValue ("radius", "Search radius of new locations", radius);
  cmd.AddValue ("width", "Width of each grid location", width);
  cmd.AddValue ("duration", "Simulation total runtime", duration);
  cmd.AddValue ("minSpeed", "Minimum speed of a node", minSpeed);
  cmd.AddValue ("maxSpeed", "Maximum speed of a node", maxSpeed);
  cmd.AddValue ("pause", "Pause time for each node", pause);
  cmd.Parse (argc, argv);

  std::string speed = "ns3::UniformRandomVariable[Min=" + minSpeed + "|Max=" + maxSpeed + "]";
  std::string max = std::to_string(width * (double)(dimension));
  double d_max = width * (double)(dimension);

  // Create nodes
  NodeContainer c;
  c.Create (numNodes);

  // Create mobility model
  MobilityHelper mobility;
  ObjectFactory pos;
  if (allocator == "uniformgrid")
  {
    pos.SetTypeId ("ns3::UniformGridPositionAllocator");
    pos.Set( "Dimension", IntegerValue (dimension));
    pos.Set( "Delta", DoubleValue (width));
    pos.Set( "Radius", IntegerValue (radius));
  } else // "rect"
  {
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + max + "]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + max + "]"));
  }
  Ptr <PositionAllocator> taPositionAlloc = pos.Create ()->GetObject <PositionAllocator> ();

  // select mobility model
  if (allocator == "uniformgrid")
  {
    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", 
                                "Speed", StringValue (speed),
                                "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=" + pause + "]"), 
                                "PositionAllocator", PointerValue (taPositionAlloc));
  }
  else if (allocator == "randomwalk")
  {
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                                "Mode", StringValue ("Time"),
                                "Time", StringValue ("2s"), 
                                "Speed", StringValue (speed),
                                "Bounds", StringValue ("0|" + max + "|0|" + max));
  }
  else if (allocator == "randomwaypoint")
  {
    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", 
                                "Speed", StringValue (speed),
                                "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=" + pause + "]"), 
                                "PositionAllocator", PointerValue (taPositionAlloc));
  }
  else if (allocator == "randomdirection")
  {
    mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel", 
                                "Bounds", RectangleValue (Rectangle (0, d_max, 0, d_max)),
                                "Speed", StringValue (speed),
                                "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=" + pause + "]"));
  }
  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.InstallAll ();

  // Run Simulation
  Simulator::Stop (Seconds (duration));

  Simulator::Run ();

  // Read and Write stats from simulation
  //std::ofstream output;
  //std::string output_name = allocator + "_data.txt";
  //output.open ("empty_cell.txt");
  //output << "node,x-coord,y-coord,distFromCenter\n";

  double average_dist_sum = 0;
  double center = width * (double)(dimension) / 2;
  
  // Initialize 2d vector cell with dimension sizes
  std::vector<std::vector<bool>> flag;
  flag.resize(dimension);
  for (uint32_t i = 0; i < dimension; i++){
    flag[i].resize(dimension);
    for (uint32_t j = 0; j < dimension; j++){
      flag[i][j] = false;
    }
  }    

  for (uint32_t i = 0; i < numNodes; i++) {
    Ptr<MobilityModel> mmp = c.Get(i)->GetObject<MobilityModel>();
    Vector apV;
    apV = mmp->GetPosition();
    //std::cout << "Node " << i << ": x-coord: " << apV.x << ", y-coord: " << apV.y << std::endl;
    double average_dist = pow(pow(((double)apV.x - center), 2) + pow(((double)apV.y - center), 2),0.5);
    average_dist_sum += average_dist;

    // Set cell at node position as visited
    flag[(int)((double)apV.x/width)][(int)((double)apV.y/width)] = true;
    //std::cout << flag[(int)((double)apV.x/dimension)][(int)((double)apV.y/dimension)];
    //output << i << "," << apV.x << "," << apV.y << "," << average_dist << std::endl;
  }
  average_dist_sum /= numNodes;
  
  // count the number of unvisited cell
  int count = 0;
  for (uint32_t i = 0; i < dimension; i++){
    for (uint32_t j = 0; j < dimension; j++){
      if (flag[i][j] == false){
        count++;
      }
    }
  }  
  
  std::cout << count << std::endl;
  //std::cout << "Average Dist From Center: " << average_dist_sum << std::endl;
  //output << count << std::endl;

  Simulator::Destroy ();
  return 0;
}
