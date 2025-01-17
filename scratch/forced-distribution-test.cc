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

using namespace ns3;
int main (int argc, char *argv[])
{
  //RngSeedManager::SetSeed(123);
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
    std::string max = std::to_string(width * (double)(dimension));
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + max + "]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + max + "]"));
  }
  Ptr <PositionAllocator> taPositionAlloc = pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", 
                              "Speed", StringValue (speed),
                              "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=" + pause + "]"), 
                              "PositionAllocator", PointerValue (taPositionAlloc));
  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.InstallAll ();

  // Run Simulation
  Simulator::Stop (Seconds (duration));

  Simulator::Run ();

  // Read and Write stats from simulation
  //std::ofstream output;
  //std::string output_name = allocator + "_data.txt";
  //output.open (output_name);
  //output << "node,x-coord,y-coord,distFromCenter\n";

  double average_dist_sum = 0;
  double center = width * (double)(dimension) / 2;

  for (uint32_t i = 0; i < numNodes; i++) {
    Ptr<MobilityModel> mmp = c.Get(i)->GetObject<MobilityModel>();
    Vector apV;
    apV = mmp->GetPosition();
    std::cout << "Node " << i << ": x-coord: " << apV.x << ", y-coord: " << apV.y << std::endl;
    double average_dist = pow(pow(((double)apV.x - center), 2) + pow(((double)apV.y - center), 2),0.5);
    average_dist_sum += average_dist;
    //output << i << "," << apV.x << "," << apV.y << "," << average_dist << std::endl;
  }
  average_dist_sum /= numNodes;
  //std::cout << "Average Dist From Center: " << average_dist_sum << std::endl;
  //output << "Average Dist From Center: " << average_dist_sum << std::endl;

  Simulator::Destroy ();
  return 0;
}
