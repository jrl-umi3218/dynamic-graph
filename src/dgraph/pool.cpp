/*
 * Copyright 2010,
 * François Bleibel,
 * Olivier Stasse,
 *
 * CNRS/AIST
 *
 * This file is part of dynamic-graph.
 * dynamic-graph is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * dynamic-graph is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.  You should
 * have received a copy of the GNU Lesser General Public License along
 * with dynamic-graph.  If not, see <http://www.gnu.org/licenses/>.
 */

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

/* --- DYNAMIC-GRAPH --- */
#include <list>
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <string>
#include "dynamic-graph/pool.h"
#include "dynamic-graph/debug.h"
#include "dynamic-graph/entity.h"

using namespace dynamicgraph;

/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */

PoolStorage::
~PoolStorage  ()
{
  dgDEBUGIN(15);

  for( Entities::iterator iter=entityMap.begin (); iter!=entityMap.end ();
       iter=entityMap.begin ())
    {
      dgDEBUG(15) << "Delete \""
		   << (iter->first) <<"\""<<std::endl;
      Entity* entity = iter->second;
      deregisterEntity(iter);
      delete (entity);
    }

  dgDEBUGOUT(15);
}



/* --------------------------------------------------------------------- */
void PoolStorage::
registerEntity( const std::string& entname,Entity* ent )
{
  Entities::iterator entkey = entityMap.find(entname);
  if( entkey != entityMap.end () ) // key does exist
    {
      throw ExceptionFactory( ExceptionFactory::OBJECT_CONFLICT,
				 "Another entity already defined with the same name. ",
				 "Entity name is <%s>.",entname.c_str () );
    }
  else
    {
      dgDEBUG(10) << "Register entity <"<< entname
		   << "> in the pool." <<std::endl;
      entityMap[entname] = ent;
    }
}

void PoolStorage::
deregisterEntity( const std::string& entname )
{
  Entities::iterator entkey = entityMap.find(entname);
  if( entkey == entityMap.end () ) // key doesnot exist
    {
      throw ExceptionFactory( ExceptionFactory::OBJECT_CONFLICT,
				 "Entity not defined yet. ",
				 "Entity name is <%s>.",entname.c_str () );
    }
  else
    {
      dgDEBUG(10) << "Deregister entity <"<< entname
		   << "> from the pool." <<std::endl;
      deregisterEntity(entkey);
    }
}

void PoolStorage::
deregisterEntity( const Entities::iterator& entity )
{
  entityMap.erase( entity );
}

Entity& PoolStorage::
getEntity( const std::string& name )
{
  dgDEBUG(25) << "Get <" << name << ">"<<std::endl;
  Entities::iterator entPtr = entityMap.find( name );
  if( entPtr == entityMap.end () )
    {
      DG_THROW ExceptionFactory( ExceptionFactory::UNREFERED_OBJECT,
				     "Unknown entity."," (while calling <%s>)",
				     name.c_str () );
    }
  else return *entPtr->second;
}

bool PoolStorage::
existEntity (const std::string& name)
{
  return entityMap.find( name ) != entityMap.end();
}
bool PoolStorage::
existEntity (const std::string& name, Entity*& ptr)
{
  Entities::iterator entPtr = entityMap.find( name );
  if( entPtr == entityMap.end () ) return false;
  else
    {
      ptr = entPtr->second;
      return true;
    }
}


void PoolStorage::
clearPlugin( const std::string& name )
{
  dgDEBUGIN(5);
  std::list<Entity*> toDelete;

  for (Entities::iterator entPtr = entityMap.begin  ();
       entPtr != entityMap.end  (); ++entPtr)
    if (entPtr->second->getClassName () == name)
      toDelete.push_back (entPtr->second);

  for (std::list< Entity* >::iterator iter = toDelete.begin  ();
       iter != toDelete.end  (); ++iter)
    delete (Entity*) *iter;
  dgDEBUGOUT(5);
}



/* --------------------------------------------------------------------- */

#include <dynamic-graph/entity.h>

#ifdef WIN32
#include <time.h>
#endif /*WIN32*/

void PoolStorage::
writeGraph(const std::string &aFileName)
{
  size_t IdxPointFound = aFileName.rfind(".");
  std::string tmp1 = aFileName.substr(0,IdxPointFound);
  size_t IdxSeparatorFound = aFileName.rfind("/");
  std::string GenericName;
  if (IdxSeparatorFound!=std::string::npos)
    GenericName = tmp1.substr(IdxSeparatorFound,tmp1.length ());
  else
    GenericName = tmp1;

  /* Reading local time */
  time_t ltime;
  ltime = time(NULL);
  struct tm ltimeformatted;
#ifdef WIN32
  localtime_s(&ltimeformatted,&ltime);
#else
  localtime_r(&ltime,&ltimeformatted);
#endif /*WIN32*/

  /* Opening the file and writing the first comment. */
  std::ofstream GraphFile (aFileName.c_str (),std::ofstream::out);
  GraphFile << "/* This graph has been automatically generated. " << std::endl;
  GraphFile << "   " << 1900+ltimeformatted.tm_year
	    << " Month: " << 1+ltimeformatted.tm_mon
	    << " Day: " << ltimeformatted.tm_mday
	    << " Time: " << ltimeformatted.tm_hour
	    << ":" << ltimeformatted.tm_min;
  GraphFile << " */" << std::endl;
  GraphFile << "digraph " << GenericName << " { ";
  GraphFile << "\t graph [ label=\"" << GenericName << "\" bgcolor = white rankdir=LR ]" << std::endl
	    << "\t node [ fontcolor = black, color = black, fillcolor = gold1, style=filled, shape=box ] ; " << std::endl;
  GraphFile << "\tsubgraph cluster_Entities { " << std::endl;

  GraphFile << "\t} " << std::endl;

  for( Entities::iterator iter=entityMap.begin ();
       iter!=entityMap.end (); ++iter)
    {
      Entity* ent = iter->second;
      GraphFile << ent->getName ()
		<<" [ label = \"" << ent->getName () << "\" ," << std::endl
		<<"   fontcolor = black, color = black, fillcolor=cyan, style=filled, shape=box ]" << std::endl;
      ent->writeGraph(GraphFile);
    }


  GraphFile << "}"<< std::endl;

  GraphFile.close ();
}

void PoolStorage::
writeCompletionList(std::ostream& os)
{
  for( Entities::iterator iter=entityMap.begin ();
       iter!=entityMap.end (); ++iter)
    {
      Entity* ent = iter->second;
      ent->writeCompletionList(os);
    }


}


void PoolStorage::
commandLine( const std::string& objectName,const std::string& functionName,
	     std::istringstream& cmdArg, std::ostream& os )
{
  dgDEBUG(15) << "Object <" << objectName<< "> function <"
	       <<functionName<<">"<<std::endl;

  if( objectName=="pool" )
    {
      if( functionName=="help" )
	{
	  os <<"Pool: " << std::endl
	     <<"  - list" << std::endl
	     << " - writegraph FileName" << std::endl;
	}
      else if( functionName=="list" )
	{
	  for( Entities::iterator iter=entityMap.begin ();
	       iter!=entityMap.end (); ++iter)
	    {
	      Entity* ent = iter->second;
	      os << ent->getName ()
		 <<" (" << ent->getClassName () << ")" << std::endl;
	    }

	}
      else if (functionName=="writegraph")
	{
	  std::string aFileName;
	  cmdArg >> aFileName;
	  writeGraph(aFileName);
	}

    }
  else
    {
      Entity& ent = getEntity(objectName);
      ent.commandLine(functionName,cmdArg,os);
    }
}

static bool
objectNameParser( std::istringstream& cmdparse,
		  std::string& objName,
		  std::string& funName  )
{
  const int SIZE=128;
  char buffer[SIZE];
  cmdparse >> std::ws;
  cmdparse.getline( buffer,SIZE,'.' );
  if(! cmdparse.good () ) // The callback is not an object method
    return false;

  objName = buffer;
  //cmdparse.getline( buffer,SIZE );
  //funName = buffer;
  cmdparse >> funName;
  return true;
}

SignalBase<int>&
PoolStorage::
getSignal( std::istringstream& sigpath )
{
  std::string objname,signame;
  if(! objectNameParser( sigpath,objname,signame ) )
    { DG_THROW ExceptionFactory( ExceptionFactory::UNREFERED_SIGNAL,
				     "Parse error in signal name" ); }

  Entity& ent = getEntity( objname );
  return ent.getSignal( signame );
}

namespace dynamicgraph {
	//! The global g_pool object.
	PoolStorage g_pool;
}

