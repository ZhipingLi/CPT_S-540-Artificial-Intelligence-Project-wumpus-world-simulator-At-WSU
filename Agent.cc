// Agent.cc
//
// Fall 2021 HW5 solution.
//
// Written by Larry Holder.

#include <iostream>
#include <list>
#include <algorithm>
#include "Agent.h"
#include "Action.h"

using namespace std;

int MySearchEngine::HeuristicFunction(SearchState* state, SearchState* goalState) {
	int cityBlock = abs(state->location.X - goalState->location.X) + abs(state->location.Y - goalState->location.Y);
	return cityBlock;
	//return 0; // not a good heuristic
}

Agent::Agent ()
{
	// Initialize new agent based on new, unknown world
	worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentHasGold = false;
	lastAction = CLIMB; // dummy action
	worldState.worldSize = 3; // HW5: somewhere between 3x3 and 9x9
	worldSizeKnown = false;
	worldState.goldLocation = Location(0,0); // unknown
	worldState.wumpusLocation = Location(0,0); // unknown
}
Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	// Initialize agent back to the beginning of the world
	worldState.agentLocation = Location(1,1);
	worldState.agentOrientation = RIGHT;
	worldState.agentAlive = true;
	worldState.agentHasGold = false;
	worldState.wumpusAlive = true;
	lastAction = CLIMB; // dummy action
	actionList.clear();
	safeLocations.remove(worldState.wumpusLocation);
	searchEngine.RemoveSafeLocation(worldState.wumpusLocation.X,worldState.wumpusLocation.Y);
	visitedLocations.clear();
}

Action Agent::Process (Percept& percept)
{
	list<Action> actionList2;
		UpdateState(lastAction, percept);

		cout << "______________________________________________________\n\n";
		cout << " StechLocations --->";
		for(int i=0;i<worldState.stenchLocations.size();i++){
			cout << " (" << worldState.stenchLocations[i].X << "," << worldState.stenchLocations[i].Y << ")";
		}
		list<Location>::iterator it, it1, it2, it3;
		cout << "\n searchEngine SafeLocations --->";
		for(it=searchEngine.safeLocations.begin();it!=searchEngine.safeLocations.end();it++){
			cout << " (" << (*it).X << "," << (*it).Y << ")";
		}
		cout << "\n SafeLocations --->";
		for(it3=searchEngine.safeLocations.begin();it3!=searchEngine.safeLocations.end();it3++){
			cout << " (" << (*it3).X << "," << (*it3).Y << ")";
		}
		cout << "\n UnsafeLocations --->";
		for(it1=unsafeLocations.begin();it1!=unsafeLocations.end();it1++){
			cout << " (" << (*it1).X << "," << (*it1).Y << ")";
		}
		cout << "\n visitedLocations --->";
		for(it2=visitedLocations.begin();it2!=visitedLocations.end();it2++){
			cout << " (" << (*it2).X << "," << (*it2).Y << ")";
		}
		cout << "\n WumpusLocation ---> (" << worldState.wumpusLocation.X << "," << worldState.wumpusLocation.Y << ").";
		cout << "\n wumpusAlive ---> (" << worldState.wumpusAlive << ").\n";
		cout << "______________________________________________________\n";

		if (actionList.empty()) {
			if (percept.Glitter) {
				// HW5.4: If there is gold, then GRAB it
				cout << "Found gold. Grabbing it.\n";
				actionList.push_back(GRAB);
			} else if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1))) {
				// HW5.5: If agent has gold and is in (1,1), then CLIMB
				cout << "Have gold and in (1,1). Climbing.\n";
				actionList.push_back(CLIMB);
			} else if (!worldState.agentHasGold && !(worldState.goldLocation == Location(0,0))) {
				// HW5.6: If agent doesn't have gold, but knows its location, then navigate to that location
				cout << "Moving to known gold location (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ").\n";
				actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, worldState.goldLocation, worldState.agentOrientation);
				if (!(actionList2.empty())) {
					actionList.splice(actionList.end(), actionList2);
				}else{
					goto FINDSAFEUNVISITED;
				}
			} else if (worldState.agentHasGold && !(worldState.agentLocation == Location(1,1))) {
				// HW5.7: If agent has gold, but isn't in (1,1), then navigate to (1,1)
				cout << "Have gold. Moving to (1,1).\n";
				actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), worldState.agentOrientation);
				actionList.splice(actionList.end(), actionList2);
			} else {
				FINDSAFEUNVISITED:
				// HW5.8: If safe unvisited location, then navigate there (should be one)
				Location safeUnvisitedLocation = SafeUnvisitedLocation();
				cout << "Moving to safe unvisited location (" << safeUnvisitedLocation.X << "," << safeUnvisitedLocation.Y << ").\n";
				actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, safeUnvisitedLocation, worldState.agentOrientation);
				if (!(actionList2.empty())) {
					actionList.splice(actionList.end(), actionList2);
				} else {
					if(worldState.wumpusAlive && !(worldState.wumpusLocation == Location(0,0))){ //Wumpus position is known
						Location facingWumpusPosition = Location(0,0);
						Orientation facingWumpusOrientation;
						list<Location>::iterator itor;
						for(itor=safeLocations.begin();itor!=safeLocations.end();itor++){
							if((*itor).X == worldState.wumpusLocation.X && (*itor).Y >= worldState.wumpusLocation.Y){
								facingWumpusPosition = *itor;
								facingWumpusOrientation = DOWN;
								break;
							}else if((*itor).X == worldState.wumpusLocation.X && (*itor).Y <= worldState.wumpusLocation.Y){
								facingWumpusPosition = *itor;
								facingWumpusOrientation = UP;
								break;
							}else if((*itor).Y == worldState.wumpusLocation.Y && (*itor).X >= worldState.wumpusLocation.X){
								facingWumpusPosition = *itor;
								facingWumpusOrientation = LEFT;
								break;
							}else if((*itor).Y == worldState.wumpusLocation.Y && (*itor).X <= worldState.wumpusLocation.X){
								facingWumpusPosition = *itor;
								facingWumpusOrientation = RIGHT;
								break;
							}
						}
						if(!(facingWumpusPosition == Location(0,0))){ // There is a safe location facing the Wumpus
							// Agent moves there and shoots the Wumpus.
							actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, facingWumpusPosition, facingWumpusOrientation);
							if (!(actionList2.empty())) {
								actionList2.push_back(SHOOT);
							}
						}
					}
					if ((actionList2.empty())) {
						actionList2 = findAUnvisitedPosition(percept);
					}
					actionList.splice(actionList.end(), actionList2);
				}
			}
		}
		Action action = actionList.front();
		actionList.pop_front();
		lastAction = action;
		return action;
}

void Agent::GameOver (int score)
{
	if (score < -1000) {
		// Agent died by going forward into pit or Wumpus
		Percept percept; // dummy, values don't matter
		UpdateState(GOFORWARD, percept, true);
		int X = worldState.agentLocation.X;
		int Y = worldState.agentLocation.Y;
		if (!(MemberLocation(worldState.agentLocation, unsafeLocations))) {
			unsafeLocations.push_back(worldState.agentLocation);
		}
		safeLocations.remove(worldState.agentLocation);
		searchEngine.RemoveSafeLocation(X,Y);
		cout << "Found unsafe location at (" << X << "," << Y << ")\n";
	}
}

void Agent::UpdateState(Action lastAction, Percept& percept, bool gameOver) {
	int X = worldState.agentLocation.X;
	int Y = worldState.agentLocation.Y;
	Orientation orientation = worldState.agentOrientation;

	if (lastAction == TURNLEFT) {
		worldState.agentOrientation = (Orientation) ((orientation + 1) % 4);
	}
	if (lastAction == TURNRIGHT) {
		if (orientation == RIGHT) {
			worldState.agentOrientation = DOWN;
		} else {
			worldState.agentOrientation = (Orientation) (orientation - 1);
		}
	}
	if (lastAction == GOFORWARD) {
		if (percept.Bump) {
			if ((orientation == RIGHT) || (orientation == UP)) {
				cout << "World size known to be " << worldState.worldSize << "x" << worldState.worldSize << endl;
				worldSizeKnown = true;
				RemoveOutsideLocations();
			}
		} else {
			switch (orientation) {
			case UP:
				worldState.agentLocation.Y = Y + 1;
				break;
			case DOWN:
				worldState.agentLocation.Y = Y - 1;
				break;
			case LEFT:
				worldState.agentLocation.X = X - 1;
				break;
			case RIGHT:
				worldState.agentLocation.X = X + 1;
				break;
			}
		}
	}
	if (lastAction == GRAB) { // Assume GRAB only done if Glitter was present
		worldState.agentHasGold = true;
	}
	if (lastAction == CLIMB) {
		// do nothing; if CLIMB worked, this won't be executed anyway
	}
	// HW5 requirement 3a
	if (percept.Glitter) {
		worldState.goldLocation = worldState.agentLocation;
		cout << "Found gold at (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")\n";
	}

	if(percept.Scream){
		unsafeLocations.remove(worldState.wumpusLocation);
		if (!(MemberLocation(worldState.wumpusLocation, safeLocations))) {
			safeLocations.push_back(worldState.wumpusLocation);
		}
		searchEngine.AddSafeLocation(worldState.wumpusLocation.X, worldState.wumpusLocation.Y);
		worldState.wumpusAlive = false;
	}

	// HW5 clarification: track world size
	int new_max = max(worldState.agentLocation.X, worldState.agentLocation.Y);
	if (new_max > worldState.worldSize) {
		worldState.worldSize = new_max;
	}
	// HW5 requirement 3b
	if (!gameOver) {
		UpdateSafeLocations(worldState.agentLocation);
	}
	// HW5 requirement 3c
	if (!(MemberLocation(worldState.agentLocation, visitedLocations))) {
		visitedLocations.push_back(worldState.agentLocation);
		// Update the Stench positions
		if (percept.Stench) {
			bool isExist = false;
			for(int i = 0;i<worldState.stenchLocations.size();i++){
				if(worldState.agentLocation == worldState.stenchLocations[i]){
					isExist = true;
					break;
				}
			}
			if(!isExist){
				worldState.stenchLocations.push_back (worldState.agentLocation);
			}
		}
	}
	if (worldState.wumpusAlive){
		if(!(worldState.wumpusLocation == Location(0,0))){ //Wumpus position is known
			if (!(MemberLocation(worldState.wumpusLocation, unsafeLocations))) {
				unsafeLocations.push_back(worldState.wumpusLocation);
			}
			list<Location> adj_wumpus;
			list<Location>::iterator wumpus_itr;
			AdjacentLocations(worldState.wumpusLocation, adj_wumpus);
			for (wumpus_itr = adj_wumpus.begin(); wumpus_itr != adj_wumpus.end(); ++wumpus_itr) {
				if ((!(MemberLocation(*wumpus_itr, safeLocations))) && (!(MemberLocation(*wumpus_itr, unsafeLocations)))) {
					safeLocations.push_back(*wumpus_itr);
					searchEngine.AddSafeLocation(wumpus_itr->X, wumpus_itr->Y);
				}
			}
		}else{
			//Update the Wumpus position
			for(int i = 0;i<worldState.stenchLocations.size();i++){
				for(int j = 0;j<worldState.stenchLocations.size();j++){
					if(worldState.stenchLocations[i].X - worldState.stenchLocations[j].X == 1 && worldState.stenchLocations[i].Y - worldState.stenchLocations[j].Y == 1){
						if(searchEngine.SafeLocation(worldState.stenchLocations[i].X, worldState.stenchLocations[i].Y - 1)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[i].X - 1, worldState.stenchLocations[i].Y);
						}else if(searchEngine.SafeLocation(worldState.stenchLocations[i].X - 1, worldState.stenchLocations[i].Y)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[i].X, worldState.stenchLocations[i].Y - 1);
						}
					}else if(worldState.stenchLocations[j].X - worldState.stenchLocations[i].X == 1 && worldState.stenchLocations[j].Y - worldState.stenchLocations[i].Y == 1){
						if(searchEngine.SafeLocation(worldState.stenchLocations[j].X, worldState.stenchLocations[j].Y - 1)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[j].X - 1, worldState.stenchLocations[j].Y);
						}else if(searchEngine.SafeLocation(worldState.stenchLocations[j].X - 1, worldState.stenchLocations[j].Y)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[j].X, worldState.stenchLocations[j].Y - 1);
						}
					}else if(worldState.stenchLocations[i].X - worldState.stenchLocations[j].X == -1 && worldState.stenchLocations[i].Y - worldState.stenchLocations[j].Y == 1){
						if(searchEngine.SafeLocation(worldState.stenchLocations[i].X + 1, worldState.stenchLocations[i].Y)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[i].X, worldState.stenchLocations[i].Y - 1);
						}else if(searchEngine.SafeLocation(worldState.stenchLocations[i].X, worldState.stenchLocations[i].Y - 1)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[i].X + 1, worldState.stenchLocations[i].Y);
						}
					}else if(worldState.stenchLocations[j].X - worldState.stenchLocations[i].X == -1 && worldState.stenchLocations[j].Y - worldState.stenchLocations[i].Y == 1){
						if(searchEngine.SafeLocation(worldState.stenchLocations[j].X + 1, worldState.stenchLocations[j].Y)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[j].X, worldState.stenchLocations[j].Y - 1);
						}else if(searchEngine.SafeLocation(worldState.stenchLocations[j].X, worldState.stenchLocations[j].Y - 1)){
							worldState.wumpusLocation = Location(worldState.stenchLocations[j].X + 1, worldState.stenchLocations[j].Y);
						}
					}
				}
			}
		}
	}
}

bool Agent::MemberLocation(Location& location, list<Location>& locationList) {
	if (find(locationList.begin(), locationList.end(), location) != locationList.end()) {
		return true;
	}
	return false;
}

Location Agent::SafeUnvisitedLocation() {
	list<Location>::iterator loc_itr;
	// Find and return safe unvisited location.
	for (loc_itr = safeLocations.begin(); loc_itr != safeLocations.end(); ++loc_itr) {
		if (!(MemberLocation(*loc_itr, visitedLocations))) {
			return *loc_itr;
		}
	}
	return Location(0,0);
}

void Agent::UpdateSafeLocations(Location& location) {
	// HW5 requirement 3b, and HW5 clarification about not known to be unsafe locations
    // Add current and adjacent locations to safe locations, unless known to be unsafe.
	if (!(MemberLocation(location, safeLocations))) {
		safeLocations.push_back(location);
		searchEngine.AddSafeLocation(location.X, location.Y);
	}
	list<Location> adj_locs;
	list<Location>::iterator loc_itr;
	AdjacentLocations(location, adj_locs);
	for (loc_itr = adj_locs.begin(); loc_itr != adj_locs.end(); ++loc_itr) {
		if ((!(MemberLocation(*loc_itr, safeLocations))) && (!(MemberLocation(*loc_itr, unsafeLocations)))) {
			safeLocations.push_back(*loc_itr);
			searchEngine.AddSafeLocation(loc_itr->X, loc_itr->Y);
		}
	}
}

void Agent::RemoveOutsideLocations() {
	// Know exact world size, so remove locations outside the world.
	int boundary = worldState.worldSize + 1;
	for (int i = 1; i < boundary; ++i) {
		safeLocations.remove(Location(i,boundary));
		searchEngine.RemoveSafeLocation(i,boundary);
		safeLocations.remove(Location(boundary,i));
		searchEngine.RemoveSafeLocation(boundary,i);
	}
	safeLocations.remove(Location(boundary,boundary));
	searchEngine.RemoveSafeLocation(boundary,boundary);
}

void Agent::AdjacentLocations(Location& location, list<Location>& adjacentLocations) {
	// Append locations adjacent to given location on to give locations list.
	// One row/col beyond unknown world size is okay. Locations outside the world
	// will be removed later.
	int X = location.X;
	int Y = location.Y;
	if (X > 1) {
		adjacentLocations.push_back(Location(X-1,Y));
	}
	if (Y > 1) {
		adjacentLocations.push_back(Location(X,Y-1));
	}
	if (worldSizeKnown) {
		if (X < worldState.worldSize) {
			adjacentLocations.push_back(Location(X+1,Y));
		}
		if (Y < worldState.worldSize) {
			adjacentLocations.push_back(Location(X,Y+1));
		}
	} else {
		adjacentLocations.push_back(Location(X+1,Y));
		adjacentLocations.push_back(Location(X,Y+1));
	}
}

list<Action> Agent::findAUnvisitedPosition(Percept& percept){
	Orientation orientation = worldState.agentOrientation;
	list<Action> actionList2;
	Location location1 = Location(worldState.agentLocation.X, worldState.agentLocation.Y + 1);
	Location location2 = Location(worldState.agentLocation.X - 1, worldState.agentLocation.Y);
	Location location3 = Location(worldState.agentLocation.X + 1, worldState.agentLocation.Y);
	Location location4 = Location(worldState.agentLocation.X, worldState.agentLocation.Y - 1);
	if(!(MemberLocation(location1,unsafeLocations)) && !(MemberLocation(location1,visitedLocations)) && (!percept.Bump || orientation != UP)){ 
		switch (orientation)
		{
		case UP:
			actionList2.push_back(GOFORWARD);
			break;
		case LEFT:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		case RIGHT:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		case DOWN:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		}
	}else if(!(MemberLocation(location3,unsafeLocations)) && !(MemberLocation(location3,visitedLocations)) && (!percept.Bump || orientation != RIGHT)){
		switch (orientation)
		{
		case UP:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		case LEFT:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		case RIGHT:
			actionList2.push_back(GOFORWARD);
			break;
		case DOWN:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		}
	}else if(!(MemberLocation(location2,unsafeLocations)) && !(MemberLocation(location2,visitedLocations)) && (!percept.Bump || orientation != LEFT) && worldState.agentLocation.X > 1){ 
		switch (orientation)
		{
		case UP:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		case LEFT:
			actionList2.push_back(GOFORWARD);
			break;
		case RIGHT:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		case DOWN:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		}
	}else if(!(MemberLocation(location4,unsafeLocations)) && !(MemberLocation(location4,visitedLocations)) && (!percept.Bump || orientation != DOWN) && worldState.agentLocation.Y > 1){
		switch (orientation)
		{
		case UP:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		case LEFT:
			actionList2.push_back(TURNLEFT);
			actionList2.push_back(GOFORWARD);
			break;
		case RIGHT:
			actionList2.push_back(TURNRIGHT);
			actionList2.push_back(GOFORWARD);
			break;
		case DOWN:
			actionList2.push_back(GOFORWARD);
			break;
		}
	}
	if (actionList2.empty()) {
		Action action = (Action) (rand() % 3);
		actionList2.push_back(action);
	}
	return actionList2;
}

