/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "position-allocator.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/csv-reader.h"

#include <cmath>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PositionAllocator");

NS_OBJECT_ENSURE_REGISTERED (PositionAllocator);

TypeId
PositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PositionAllocator")
    .SetParent<Object> ()
    .SetGroupName ("Mobility");
  return tid;
}

PositionAllocator::PositionAllocator ()
{}

PositionAllocator::~PositionAllocator ()
{}


NS_OBJECT_ENSURE_REGISTERED (ListPositionAllocator);

TypeId
ListPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ListPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<ListPositionAllocator> ()
    ;
  return tid;
}

ListPositionAllocator::ListPositionAllocator ()
{}

void
ListPositionAllocator::Add (Vector v)
{
  m_positions.push_back (v);
  m_current = m_positions.begin ();
}

void
ListPositionAllocator::Add (const std::string filePath,
                            double defaultZ /* = 0 */,
                            char delimiter /* = ',' */)
{
  NS_LOG_FUNCTION (this << filePath << std::string ("'") + delimiter + "'");

  CsvReader csv (filePath, delimiter);
  while (csv.FetchNextRow ())
    {
      if (csv.ColumnCount () == 1)
        {
          // comment line
          continue;
        }

      double x, y, z;
      bool ok = csv.GetValue (0, x);
      NS_LOG_INFO ("read x: " << x << (ok ? " ok" : " FAIL"));
      NS_ASSERT_MSG (ok, "failed reading x");
      ok = csv.GetValue (1, y);
      NS_LOG_INFO ("read y = " << y << (ok ? " ok" : " FAIL"));
      NS_ASSERT_MSG (ok, "failed reading y");
      if (csv.ColumnCount () > 2)
        {
          ok = csv.GetValue (2, z);
          NS_LOG_INFO ("read z = " << z << (ok ? " ok" : " FAIL"));
          NS_ASSERT_MSG (ok, "failed reading z");
        }
      else
        {
          z = defaultZ;
          NS_LOG_LOGIC ("using default Z " << defaultZ);
        }

      Vector pos (x, y, z);
      Add (pos);

    }  // while FetchNextRow
  NS_LOG_INFO ("read " << csv.RowNumber () << " rows");
}

Vector
ListPositionAllocator::GetNext (void) const
{
  Vector v = *m_current;
  m_current++;
  if (m_current == m_positions.end ())
    {
      m_current = m_positions.begin ();
    }
  return v;
}

int64_t
ListPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

uint32_t
ListPositionAllocator::GetSize (void) const
{
  return m_positions.size ();
}


NS_OBJECT_ENSURE_REGISTERED (GridPositionAllocator);

TypeId
GridPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GridPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<GridPositionAllocator> ()
    .AddAttribute ("GridWidth", "The number of objects laid out on a line.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&GridPositionAllocator::m_n),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinX", "The x coordinate where the grid starts.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_xMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinY", "The y coordinate where the grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_yMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions allocated.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DeltaX", "The x space between objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_deltaX),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DeltaY", "The y space between objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_deltaY),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LayoutType", "The type of layout.",
                   EnumValue (ROW_FIRST),
                   MakeEnumAccessor (&GridPositionAllocator::m_layoutType),
                   MakeEnumChecker (ROW_FIRST, "RowFirst",
                                    COLUMN_FIRST, "ColumnFirst"))
    ;
  return tid;
}

GridPositionAllocator::GridPositionAllocator ()
  : m_current (0)
{}

void
GridPositionAllocator::SetMinX (double xMin)
{
  m_xMin = xMin;
}

void
GridPositionAllocator::SetMinY (double yMin)
{
  m_yMin = yMin;
}

void
GridPositionAllocator::SetZ (double z)
{
  m_z = z;
}

void
GridPositionAllocator::SetDeltaX (double deltaX)
{
  m_deltaX = deltaX;
}

void
GridPositionAllocator::SetDeltaY (double deltaY)
{
  m_deltaY = deltaY;
}

void
GridPositionAllocator::SetN (uint32_t n)
{
  m_n = n;
}

void
GridPositionAllocator::SetLayoutType (enum LayoutType layoutType)
{
  m_layoutType = layoutType;
}

double
GridPositionAllocator::GetMinX (void) const
{
  return m_xMin;
}

double
GridPositionAllocator::GetMinY (void) const
{
  return m_yMin;
}

double
GridPositionAllocator::GetDeltaX (void) const
{
  return m_deltaX;
}

double
GridPositionAllocator::GetDeltaY (void) const
{
  return m_deltaY;
}

uint32_t
GridPositionAllocator::GetN (void) const
{
  return m_n;
}

enum GridPositionAllocator::LayoutType
GridPositionAllocator::GetLayoutType (void) const
{
  return m_layoutType;
}

Vector
GridPositionAllocator::GetNext (void) const
{
  double x = 0.0, y = 0.0;
  switch (m_layoutType)
    {
      case ROW_FIRST:
        x = m_xMin + m_deltaX * (m_current % m_n);
        y = m_yMin + m_deltaY * (m_current / m_n);
        break;
      case COLUMN_FIRST:
        x = m_xMin + m_deltaX * (m_current / m_n);
        y = m_yMin + m_deltaY * (m_current % m_n);
        break;
    }
  m_current++;
  return Vector (x, y, m_z);
}

int64_t
GridPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (UniformGridPositionAllocator);

TypeId
UniformGridPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UniformGridPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UniformGridPositionAllocator> ()
    .AddAttribute ("Dimension", "The dimensions for both the x and y axis",
                   UintegerValue (6),
                   MakeIntegerAccessor(&UniformGridPositionAllocator::m_dimension),
                   MakeIntegerChecker<uint32_t> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions allocated.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformGridPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Delta", "The space between objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&UniformGridPositionAllocator::m_delta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Radius", "Search radius to determine a node's new position",
                   IntegerValue (1),
                   MakeIntegerAccessor (&UniformGridPositionAllocator::m_radius),
                   MakeIntegerChecker<int32_t> ())
    ;
  return tid;
}

UniformGridPositionAllocator::UniformGridPositionAllocator ()
  : m_current (0)
{
}

void
UniformGridPositionAllocator::SetDimension(int32_t dimension)
{
  m_dimension = dimension;
}

void
UniformGridPositionAllocator::SetZ (double z)
{
  m_z = z;
}

void
UniformGridPositionAllocator::SetDelta (double delta)
{
  m_delta = delta;
}

void
UniformGridPositionAllocator::SetRadius (int32_t radius)
{
  m_radius = radius;
}

void
UniformGridPositionAllocator::SetCoords (std::pair<double, double> coords)
{
  m_coords = coords;
}

int32_t
UniformGridPositionAllocator::GetDimension (void) const
{
  return m_dimension;
}

double
UniformGridPositionAllocator::GetDelta (void) const
{
  return m_delta;
}

int32_t
UniformGridPositionAllocator::GetRadius (void) const
{
  return m_radius;
}

Vector
UniformGridPositionAllocator::GetGridVector (int32_t grid_num) const
{
  // Determine row and column number
  int row = grid_num / m_dimension;
  int col = grid_num % m_dimension;

  // Determine coordinate values
  double x = col * m_delta + (m_delta / 2.0);
  double y = row * m_delta + (m_delta / 2.0);

  // Debug Statements
  //std::cout << "Old Coords: " << m_coords.first << ',' << m_coords.second << std::endl;
  //std::cout << "New Coords: " << x << ',' << y << std::endl << std::endl;
  //std::cout << "Old Node: " << GetGridNumber(m_coords.first, m_coords.second) << " New Node: " << GetGridNumber(x, y) << std::endl;

  // Return vector
  return Vector(x, y , m_z);
}

int32_t
UniformGridPositionAllocator::GetGridNumber (double x, double y) const
{
  // Convert coordinates to a grid number
  int32_t col = x / m_delta;
  int32_t row = (int)(y / m_delta) * m_dimension;

  return (col + row);
}

Vector
UniformGridPositionAllocator::GetNext (void) const
{
  // Get all other nodes around central node
  // based on search radius 
  std::vector<int> indexes;
  int32_t grid_num = GetGridNumber(m_coords.first, m_coords.second);
  if (m_radius != -1) {
    int row = grid_num / m_dimension;
    int col = grid_num % m_dimension;

    // Add all nodes surrounding the current node
    int new_col = col - 1;
    int radius_cnt = m_radius;
    while (new_col >= 0 && radius_cnt > 0) {
      int search_idx = new_col + (row * m_dimension);
      indexes.push_back(search_idx);
      int new_row = row + 1;
      int inner_radius_cnt = m_radius;
      while (new_row < m_dimension && inner_radius_cnt > 0) {
        int search_idx = new_col + (new_row * m_dimension);
        indexes.push_back(search_idx);
        new_row++;
        inner_radius_cnt--;
      }
      new_row = row - 1;
      inner_radius_cnt = m_radius;
      while (new_row >= 0 && inner_radius_cnt > 0) {
        int search_idx = new_col + (new_row * m_dimension);
        indexes.push_back(search_idx);
        new_row--;
        inner_radius_cnt--;
      }
      new_col--;
      radius_cnt--;
    }
    new_col = col + 1;
    radius_cnt = m_radius;
    while (new_col < m_dimension && radius_cnt > 0) {
      int search_idx = new_col + (row * m_dimension);
      indexes.push_back(search_idx);
      int new_row = row + 1;
      int inner_radius_cnt = m_radius;
      while (new_row < m_dimension && inner_radius_cnt > 0) {
        int search_idx = new_col + (new_row * m_dimension);
        indexes.push_back(search_idx);
        new_row++;
        inner_radius_cnt--;
      }
      new_row = row - 1;
      inner_radius_cnt = m_radius;
      while (new_row >= 0 && inner_radius_cnt > 0) {
        int search_idx = new_col + (new_row * m_dimension);
        indexes.push_back(search_idx);
        new_row--;
        inner_radius_cnt--;
      }
      new_col++;
      radius_cnt--;

    }
    int new_row = row + 1;
    radius_cnt = m_radius;
    while (new_row < m_dimension && radius_cnt > 0) {
      int search_idx = col + (new_row * m_dimension);
      indexes.push_back(search_idx);
      new_row++;
      radius_cnt--;
    }
    new_row = row - 1;
    radius_cnt = m_radius;
    while (new_row >= 0 && radius_cnt > 0) {
      int search_idx = col + (new_row * m_dimension);
      indexes.push_back(search_idx);
      new_row--;
      radius_cnt--;
    }
  } else { // Otherwise, add all locations
    for (int i = 0; i < m_dimension * m_dimension; i++) {
      if (i == grid_num) continue;
      indexes.push_back(i);
    }
  }


  // Find minimum value in map first
  int min = 999999;
  for (auto i : indexes) {
    if (grid_visits[i] < min) min = grid_visits[i];
  }

  std::vector<int> min_indexes;
  // Search through entire map and keep track of locations with lowest visit count
  for (auto i : indexes) {
    if (grid_visits[i] == min) min_indexes.push_back(i);
  }
  
  // Based on size of vector, choose random grid location
  int lowest_visit = min_indexes[(int)(rand_num->GetValue() * min_indexes.size())];

  // Increment visit count
  grid_visits[lowest_visit]++;
  //std::cout << "Grid " << lowest_visit << " has visits: " << grid_visits[lowest_visit] << std::endl;
  
  // Return Vector
  return GetGridVector(lowest_visit);
}

int64_t
UniformGridPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}


NS_OBJECT_ENSURE_REGISTERED (RandomRectanglePositionAllocator);

TypeId
RandomRectanglePositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomRectanglePositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomRectanglePositionAllocator> ()
    .AddAttribute ("X",
                   "A random variable which represents the x coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomRectanglePositionAllocator::m_x),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Y",
                   "A random variable which represents the y coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomRectanglePositionAllocator::m_y),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions allocated.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomRectanglePositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

RandomRectanglePositionAllocator::RandomRectanglePositionAllocator ()
{}

RandomRectanglePositionAllocator::~RandomRectanglePositionAllocator ()
{}

void
RandomRectanglePositionAllocator::SetX (Ptr<RandomVariableStream> x)
{
  m_x = x;
}

void
RandomRectanglePositionAllocator::SetY (Ptr<RandomVariableStream> y)
{
  m_y = y;
}

void
RandomRectanglePositionAllocator::SetZ (double z)
{
  m_z = z;
}

Vector
RandomRectanglePositionAllocator::GetNext (void) const
{
  double x = m_x->GetValue ();
  double y = m_y->GetValue ();
  return Vector (x, y, m_z);
}

int64_t
RandomRectanglePositionAllocator::AssignStreams (int64_t stream)
{
  m_x->SetStream (stream);
  m_y->SetStream (stream + 1);
  return 2;
}


NS_OBJECT_ENSURE_REGISTERED (RandomBoxPositionAllocator);

TypeId
RandomBoxPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomBoxPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomBoxPositionAllocator> ()
    .AddAttribute ("X",
                   "A random variable which represents the x coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_x),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Y",
                   "A random variable which represents the y coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_y),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Z",
                   "A random variable which represents the z coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_z),
                   MakePointerChecker<RandomVariableStream> ())
    ;
  return tid;
}

RandomBoxPositionAllocator::RandomBoxPositionAllocator ()
{}

RandomBoxPositionAllocator::~RandomBoxPositionAllocator ()
{}

void
RandomBoxPositionAllocator::SetX (Ptr<RandomVariableStream> x)
{
  m_x = x;
}

void
RandomBoxPositionAllocator::SetY (Ptr<RandomVariableStream> y)
{
  m_y = y;
}

void
RandomBoxPositionAllocator::SetZ (Ptr<RandomVariableStream> z)
{
  m_z = z;
}

Vector
RandomBoxPositionAllocator::GetNext (void) const
{
  double x = m_x->GetValue ();
  double y = m_y->GetValue ();
  double z = m_z->GetValue ();
  return Vector (x, y, z);
}

int64_t
RandomBoxPositionAllocator::AssignStreams (int64_t stream)
{
  m_x->SetStream (stream);
  m_y->SetStream (stream + 1);
  m_z->SetStream (stream + 2);
  return 3;
}


NS_OBJECT_ENSURE_REGISTERED (RandomDiscPositionAllocator);

TypeId
RandomDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomDiscPositionAllocator> ()
    .AddAttribute ("Theta",
                   "A random variable which represents the angle (gradients) of a position in a random disc.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"),
                   MakePointerAccessor (&RandomDiscPositionAllocator::m_theta),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Rho",
                   "A random variable which represents the radius of a position in a random disc.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"),
                   MakePointerAccessor (&RandomDiscPositionAllocator::m_rho),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("X",
                   "The x coordinate of the center of the random position disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_x),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Y",
                   "The y coordinate of the center of the random position disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_y),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions in the disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

RandomDiscPositionAllocator::RandomDiscPositionAllocator ()
{}

RandomDiscPositionAllocator::~RandomDiscPositionAllocator ()
{}

void
RandomDiscPositionAllocator::SetTheta (Ptr<RandomVariableStream> theta)
{
  m_theta = theta;
}

void
RandomDiscPositionAllocator::SetRho (Ptr<RandomVariableStream> rho)
{
  m_rho = rho;
}

void
RandomDiscPositionAllocator::SetX (double x)
{
  m_x = x;
}

void
RandomDiscPositionAllocator::SetY (double y)
{
  m_y = y;
}

void
RandomDiscPositionAllocator::SetZ (double z)
{
  m_z = z;
}

Vector
RandomDiscPositionAllocator::GetNext (void) const
{
  double theta = m_theta->GetValue ();
  double rho = m_rho->GetValue ();
  double x = m_x + std::cos (theta) * rho;
  double y = m_y + std::sin (theta) * rho;
  NS_LOG_DEBUG ("Disc position x=" << x << ", y=" << y);
  return Vector (x, y, m_z);
}

int64_t
RandomDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_theta->SetStream (stream);
  m_rho->SetStream (stream + 1);
  return 2;
}


NS_OBJECT_ENSURE_REGISTERED (UniformDiscPositionAllocator);

TypeId
UniformDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UniformDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UniformDiscPositionAllocator> ()
    .AddAttribute ("rho",
                   "The radius of the disc",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_rho),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("X",
                   "The x coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_x),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Y",
                   "The y coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_y),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions in the disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

UniformDiscPositionAllocator::UniformDiscPositionAllocator ()
{
  m_rv = CreateObject<UniformRandomVariable> ();
}

UniformDiscPositionAllocator::~UniformDiscPositionAllocator ()
{}

void
UniformDiscPositionAllocator::SetRho (double rho)
{
  m_rho = rho;
}

void
UniformDiscPositionAllocator::SetX (double x)
{
  m_x = x;
}

void
UniformDiscPositionAllocator::SetY (double y)
{
  m_y = y;
}

void
UniformDiscPositionAllocator::SetZ (double z)
{
  m_z = z;
}

Vector
UniformDiscPositionAllocator::GetNext (void) const
{
  double x,y;
  do
    {
      x = m_rv->GetValue (-m_rho, m_rho);
      y = m_rv->GetValue (-m_rho, m_rho);
    }
  while (std::sqrt (x * x + y * y) > m_rho);

  x += m_x;
  y += m_y;
  NS_LOG_DEBUG ("Disc position x=" << x << ", y=" << y);
  return Vector (x, y, m_z);
}

int64_t
UniformDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_rv->SetStream (stream);
  return 1;
}


} // namespace ns3
