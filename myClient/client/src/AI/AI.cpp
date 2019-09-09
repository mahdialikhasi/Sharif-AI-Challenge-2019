#include <stdlib.h>
#include "AI.h"

using namespace std;
vector<vector<int> > Mymap(31, vector<int>(31));
vector<vector<Direction> > heroPath(4);
vector<Cell> respawns(4);

void printMap(World* world){
    Map map = world->map();
    for (int row = 0; row < map.getRowNum(); row++) {
        for (int column = 0; column < map.getColumnNum(); column++) {
            Cell cell = map.getCell(row, column);
            char cur;
            if (world->getMyHero(row, column) != Hero::NULL_HERO)
                cur = static_cast<char>('0' + world->getMyHero(row, column).getId() % 10);
            else if (world->getOppHero(row, column) != Hero::NULL_HERO)
                cur = static_cast<char>('0' + world->getOppHero(row, column).getId() % 10);
            else if (cell.isWall())
                cur = '#';
            else if (cell.isInVision())
                cur = '+';
            else
                cur = '.';
            cerr << cur << ' ';

        }
        cerr << endl;
    }
}

void AI::preProcess(World *world) {
    srand(time(0));
    Map map = world->map();
    int i = 0;
    int j = 0;
    for (int row = 0; row < map.getRowNum(); row++) {
        for (int column = 0; column < map.getColumnNum(); column++) {
            Cell cell = map.getCell(row, column);
            if(cell.isInMyRespawnZone()){
                if(j < 4){
                  respawns[j] = cell;
                  j++;
                }
                Mymap[row][column] = 2;
            }else if(cell.isInObjectiveZone()){
                Mymap[row][column] = 1;
                if(i < 4){
                  heroPath[i] = world->getPathMoveDirections(respawns[i], cell);
                  i++;
                }
            }
            else if (cell.isWall())
                Mymap[row][column] = -1;
            else
                Mymap[row][column] = 0;
        }
    }
}

void AI::pick(World *world) {
    cerr << "-pick" << endl;
    static int cnt= 0;

    // switch(cnt){
    //     case 0:
    //         world->pickHero(HeroName::BLASTER);
    //         break;
    //     case 1:
    //         world->pickHero(HeroName::GUARDIAN);
    //         break;
    //     case 2:
    //         world->pickHero(HeroName::HEALER);
    //         break;
    //     case 3:
    //         world->pickHero(HeroName::SENTRY);
    //         break;
    //     default:
    //         world->pickHero(HeroName::BLASTER);
    // }
    switch(cnt){
        case 0:
            world->pickHero(HeroName::BLASTER);
            break;
        case 1:
            world->pickHero(HeroName::BLASTER);
            break;
        case 2:
            world->pickHero(HeroName::BLASTER);
            break;
        case 3:
            world->pickHero(HeroName::HEALER);
            break;
        default:
            world->pickHero(HeroName::BLASTER);
    }
    cnt++;

}

bool WAR_IN_OBJECTIVE_ZONE = true;

int targetCellRow[4];
int targetCellColumn[4];

int checkMove(World *world, vector<Hero *> my_heros, int i, Direction _dirs){
	Cell cell = my_heros[i]->getCurrentCell();
	int row = cell.getRow();
	int col = cell.getColumn();
	//cout << "--------" << endl;
	//cout << row << ";" << col << endl;
	if(_dirs == UP)
		row--;
	if(_dirs == DOWN)
		row++;
	if(_dirs == RIGHT)
		col++;
	if(_dirs == LEFT)
		col--;
	int status = 0;
	//cout << row << ";" << col << endl;
	for (int j = 0; j < 4; ++j){
		if(j != i){
			Cell cell2 = my_heros[j]->getCurrentCell();
			int row2 = cell2.getRow();
			int col2 = cell2.getColumn();
			if(row2 == row && col2 == col)
				status = 1;
		}
	}
	if(world->map().getCell(row, col).isWall())
		status = 1;
	return status;
}
void AI::move(World *world) {
	cerr << "-move" << endl;
    static int targetRefreshPeriod = 0;

    if(targetRefreshPeriod <= 0){
        srand(time(0) + world->getMyHeroes()[0]->getId());//made this so we can test two clients with this exact AI code
        for (int i=0; i<4; ++i){
            if(WAR_IN_OBJECTIVE_ZONE) {
                std::vector<Cell *> obj_list = world->map().getObjectiveZone();

                Cell * targetCell = obj_list.at(rand() % (obj_list.size() - 10) + 5);
                targetCellRow[i] = targetCell->getRow();
                targetCellColumn[i] = targetCell->getColumn();

                // move to near a target
                for(Hero* opp_hero : world->getOppHeroes()){
	                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
	                {
	                    if(opp_hero->getCurrentCell().isInObjectiveZone()){
	                    	Cell tmp = opp_hero->getCurrentCell();
	                    	int tmpRow = tmp.getRow();
	                    	int tmpCol = tmp.getColumn();
	                    	int r = rand() % 2;
	                    	if(world->getMyHeroes()[i]->getName() == HeroName::BLASTER){
	                    		int x = 2;
	                    		if(r == 0){
	                    			// distance = 3
	                    			x = 4;
	                    		}
	                    		int d = rand() % 4;
	                    		if(tmpRow - x > 0 && !world->map().getCell(tmpRow - x, tmpCol).isWall() && world->map().getCell(tmpRow - x, tmpCol).isInObjectiveZone() && d >= 0){
                    				for (int j=0; j<4; ++j){
                    					if(j != i){
                    						if(world->getMyHeroes()[j]->getCurrentCell().getRow() != world->map().getCell(tmpRow - x, tmpCol).getRow() || world->getMyHeroes()[j]->getCurrentCell().getColumn() != world->map().getCell(tmpRow - x, tmpCol).getColumn()){
                    							// move
                    							targetCellRow[i] = world->map().getCell(tmpRow - x, tmpCol).getRow();
            									targetCellColumn[i] = world->map().getCell(tmpRow - x, tmpCol).getColumn();
                    						}
                    					}
                    				}
                    			}
                    			if(tmpRow + x > 0 && !world->map().getCell(tmpRow + x, tmpCol).isWall() && world->map().getCell(tmpRow + x, tmpCol).isInObjectiveZone() && d >= 1){
                    				for (int j=0; j<4; ++j){
                    					if(j != i){
                    						if(world->getMyHeroes()[j]->getCurrentCell() != world->map().getCell(tmpRow + x, tmpCol)){
                    							// move
                    							targetCellRow[i] = world->map().getCell(tmpRow + x, tmpCol).getRow();
            									targetCellColumn[i] = world->map().getCell(tmpRow + x, tmpCol).getColumn();
                    						}
                    					}
                    				}
                    			}
                    			if(tmpCol - x > 0 && !world->map().getCell(tmpRow, tmpCol - x).isWall() && world->map().getCell(tmpRow, tmpCol - x).isInObjectiveZone() && d >= 2){
                    				for (int j=0; j<4; ++j){
                    					if(j != i){
                    						if(world->getMyHeroes()[j]->getCurrentCell() != world->map().getCell(tmpRow , tmpCol - x)){
                    							// move
                    							targetCellRow[i] = world->map().getCell(tmpRow , tmpCol - x).getRow();
            									targetCellColumn[i] = world->map().getCell(tmpRow, tmpCol - x).getColumn();
                    						}
                    					}
                    				}
                    			}
                    			if(tmpCol + x > 0 && !world->map().getCell(tmpRow, tmpCol + x).isWall() && world->map().getCell(tmpRow, tmpCol + x).isInObjectiveZone() && d >= 3){
                    				for (int j=0; j<4; ++j){
                    					if(j != i){
                    						if(world->getMyHeroes()[j]->getCurrentCell() != world->map().getCell(tmpRow , tmpCol + x)){
                    							// move
                    							targetCellRow[i] = world->map().getCell(tmpRow , tmpCol + x).getRow();
            									targetCellColumn[i] = world->map().getCell(tmpRow, tmpCol + x).getColumn();
                    						}
                    					}
                    				}
                    			}
	                    	}
	                    }
	                }
	            }
            } else {
                while (1) {
                    targetCellRow[i] = rand() % world->map().getRowNum();
                    targetCellColumn[i] = rand() % world->map().getColumnNum();

                    //Make sure the target is not a wall!
                    if (!world->map().getCell(targetCellRow[i], targetCellColumn[i]).isWall())
                        break;
                }
            }
        }
        targetRefreshPeriod = 5 * 6;// Change target locations after 10 complete moves
    } else
        targetRefreshPeriod--;

    vector<Hero *> my_heros = world->getMyHeroes();
    for(int i=0; i < 4; ++i){
        vector<Direction> _dirs = world->getPathMoveDirections(my_heros[i]->getCurrentCell().getRow(),
                                                               my_heros[i]->getCurrentCell().getColumn(),
                                                               targetCellRow[i],
                                                               targetCellColumn[i]);
        if(_dirs.size() == 0)//ALWAYS check if there is a path to that target!!!!
            continue;

        int status = checkMove(world, my_heros, i, _dirs[0]);
    	if(status == 1){
    		// move different
    		for (int i = 0; i < 4; ++i)
    		{
    			Direction tmp[4] = {UP, DOWN, RIGHT, LEFT};
    			_dirs[0] = tmp[rand() % 4];
    			
    			if (!checkMove(world, my_heros, i, _dirs[0]))
                    break;
    		}
    	}
        world->moveHero(my_heros[i]->getId(),
                        _dirs[0]);
    }
}


void AI::action(World *world) {
    cerr << "-action" << endl;

    for(Hero* my_hero : world->getMyHeroes()){
        if(my_hero->getName() == HeroName::BLASTER){
            //Find the closest bombing target
            Cell bombing_cell = Cell::NULL_CELL;
            int min_dist = 10000;
            for(Hero* opp_hero : world->getOppHeroes()){
                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
                {
                    if(min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                           my_hero->getCurrentCell())){
                        min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                            my_hero->getCurrentCell());
                        bombing_cell = opp_hero->getCurrentCell();
                    }
                }
            }
            //Perform the bombing
            if(bombing_cell != Cell::NULL_CELL) {
                world->castAbility(*my_hero, AbilityName::BLASTER_ATTACK,bombing_cell);
            }
        } else if(my_hero->getName() == HeroName::GUARDIAN){
            //Find the closest attacking target
            Cell attack_cell = Cell::NULL_CELL;
            int min_dist = 10000;
            for(Hero* opp_hero : world->getOppHeroes()){
                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
                {
                    if(min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                           my_hero->getCurrentCell())){
                        min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                            my_hero->getCurrentCell());
                        attack_cell = opp_hero->getCurrentCell();
                    }
                }
            }
            //Perform the attack
            if(attack_cell != Cell::NULL_CELL) {
                world->castAbility(*my_hero, AbilityName::GUARDIAN_ATTACK,attack_cell);
            }
        } else if(my_hero->getName() == HeroName::SENTRY){
            //Find the closest shooting target
            Cell shoot_cell = Cell::NULL_CELL;
            int min_dist = 10000;
            for(Hero* opp_hero : world->getOppHeroes()){
                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
                {
                    if(min_dist > world->manhattanDistance(opp_hero->getCurrentCell(),
                                                           my_hero->getCurrentCell())){
                        min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
                                                            my_hero->getCurrentCell());
                        shoot_cell = opp_hero->getCurrentCell();
                    }
                }
            }
            //Perform the shooting
            if(shoot_cell != Cell::NULL_CELL) {
                world->castAbility(*my_hero, AbilityName::SENTRY_RAY,shoot_cell);
            }
        } else if(my_hero->getName() == HeroName::HEALER){
            //Find the closest healing target
            Cell target_heal_cell = Cell::NULL_CELL;
            int min_dist = 10000;
            for(Hero* _hero : world->getMyHeroes()){
                if(min_dist > world->manhattanDistance(_hero->getCurrentCell(), my_hero->getCurrentCell()) &&
                        _hero->getRemRespawnTime() == 0 &&
                        _hero->getCurrentHP() != _hero->getMaxHP() && _hero->getName() != HeroName::HEALER){
                    min_dist = world->manhattanDistance(_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell());
                    target_heal_cell = _hero->getCurrentCell();
                }
            }
            if(min_dist > 4 || (my_hero->getCurrentCell().isInObjectiveZone() && my_hero->getCurrentHP() < 100)) // heal himself
            	target_heal_cell = my_hero->getCurrentCell();
            //Do the attack
            if(target_heal_cell != Cell::NULL_CELL) {
                world->castAbility(*my_hero, AbilityName::HEALER_HEAL,target_heal_cell);
            }
        }
    }


    printMap(world);
}