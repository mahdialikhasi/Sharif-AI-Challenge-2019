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
    static int turn = 0;
    if(targetRefreshPeriod <= 0){
        srand(time(0) + world->getMyHeroes()[0]->getId());	//made this so we can test two clients with this exact AI code
        for (int i = 0; i < 4; ++i){
            if(WAR_IN_OBJECTIVE_ZONE) {
                int random = 0;
                if(world->getMyHeroes()[i]->getCurrentCell().isInObjectiveZone()){
                	random = rand() % 10;
                }
                if(random < 4){
                	std::vector<Cell *> obj_list = world->map().getObjectiveZone();

                	Cell * targetCell = obj_list.at(rand() % (obj_list.size() - 10) + 5);
                	targetCellRow[i] = targetCell->getRow();
                	targetCellColumn[i] = targetCell->getColumn();	
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
        targetRefreshPeriod = 5 * 6;// Change target locations after 5 complete moves
    } else
        targetRefreshPeriod--;

    if(turn % 6 == 0){
    	for (int i = 0; i < 4; ++i){
	    	if(world->getMyHeroes()[i]->getName() == HeroName::BLASTER){
	        	// move to near a target
	        	// find the weackest enemy in objective zone
	        	int hp = 1000;
	        	Cell enemyCell = Cell::NULL_CELL;
	        	for(Hero* opp_hero : world->getOppHeroes()){
	        		if(opp_hero->getCurrentCell().isInVision() && opp_hero->getCurrentCell().isInObjectiveZone()){
	        			if(opp_hero->getCurrentHP() < hp){
	        				hp = opp_hero->getCurrentHP();
	        				enemyCell = opp_hero->getCurrentCell();
	        			}
	        		}
	        	}
	        	if(enemyCell != Cell::NULL_CELL){
	        		// go near that target
	        		if(world->manhattanDistance(enemyCell, world->getMyHeroes()[i]->getCurrentCell()) > 4){
	        			std::vector<Cell *> obj_list = world->map().getObjectiveZone();
	        			vector<Cell *> appropriate;
	        			vector<Cell *> best;
	        			for (int k = 0; k < obj_list.size(); ++k){
	        			 	if(world->manhattanDistance(*obj_list[k], enemyCell) <= 4){
	        			 		appropriate.push_back(obj_list[k]);
	        			 	}
	        			}
	        			int status = 0;
	        			// جایی برو که کمی فاصله با بقیه ی خانه های خودی داشته باشد
	        			for (int k = 0; k < appropriate.size(); ++k){
	        				int p = 1;
	        				for (int j = 0; j < 4; ++j){
	        					if(world->manhattanDistance(appropriate[k]->getRow(), appropriate[k]->getColumn(), targetCellRow[j], targetCellColumn[j]) < 2){
	        						p = 0;
	        						break;
	        					}
	        				}
	        				if(p == 1){
	        					
	        					status = 1;
	        					best.push_back(appropriate[k]);
	        				}
	        			}
	        			if(status == 0){
	        				for (int k = 0; k < appropriate.size(); ++k){
	        					int p = 1;
	            				for (int j = 0; j < 4; ++j){
	            					if(world->manhattanDistance(appropriate[k]->getRow(), appropriate[k]->getColumn(), targetCellRow[j], targetCellColumn[j]) <= 1){
	            						p = 0;
	            						break;
	            					}
	            				}
	            				if(p == 1){
	            					targetCellRow[i] = appropriate[k]->getRow();
	            					targetCellColumn[i] = appropriate[k]->getColumn();
	            					status = 1;
	            					break;
	            				}
	            			}
	        			}else{
	        				int distance = 1000;
	        				Cell moveCell = Cell::NULL_CELL;
	        				for (int k = 0; k < best.size(); ++k){
	        					if(world->manhattanDistance(world->getMyHeroes()[i]->getCurrentCell(), *best[k]) < distance){
	        						distance = world->manhattanDistance(world->getMyHeroes()[i]->getCurrentCell(), *best[k]);
	        						moveCell = *best[k];
	        					}
	        				}
	        				targetCellRow[i] = moveCell.getRow();
	        				targetCellColumn[i] = moveCell.getColumn();
	        			}
	        		}else{
	        			//همونجا بمون
	        			targetCellRow[i] = world->getMyHeroes()[i]->getCurrentCell().getRow();
	        			targetCellColumn[i] = world->getMyHeroes()[i]->getCurrentCell().getColumn();
	        		}
	        	}
	        }
	        if(world->getMyHeroes()[i]->getName() == HeroName::HEALER){
	        	// move near the weakest hero
	        	int distance = 1000;
	        	Cell heroCell = Cell::NULL_CELL;
	        	for (int j = 0; j < 4; ++j){
	        		if(world->getMyHeroes()[j]->getCurrentHP() < world->getMyHeroes()[j]->getMaxHP() / 2){
	        			if(world->getMyHeroes()[j]->getCurrentCell().isInObjectiveZone()){
	        				if(distance > world->manhattanDistance(world->getMyHeroes()[j]->getCurrentCell(), world->getMyHeroes()[i]->getCurrentCell())){
	        					distance = world->manhattanDistance(world->getMyHeroes()[j]->getCurrentCell(), world->getMyHeroes()[i]->getCurrentCell());
	        					cout << "distance :: " << distance << endl;
	        					cout << "id : " << world->getMyHeroes()[j]->getId() << endl;
	        					heroCell = world->getMyHeroes()[j]->getCurrentCell();
	        				}
	        			}
	        		}
	        	}
	        	if(heroCell != Cell::NULL_CELL){
	        		if(world->manhattanDistance(world->getMyHeroes()[i]->getCurrentCell(), heroCell) > 4){
		        		std::vector<Cell *> obj_list = world->map().getObjectiveZone();
		        		for (int j = 0; j < obj_list.size(); ++j)
		        		{
		        			if(world->manhattanDistance(*obj_list[j], heroCell) <= 4){
		        				int status = 1;
		        				Cell targetCell = Cell::NULL_CELL;
		        				for (int k = 0; k < 4; ++k){
		        					if(i != k){
		        						if(*obj_list[j] == world->getMyHeroes()[k]->getCurrentCell()){
		        							status = 0;
		        						}
		        					}
		        				}
		        				if(status == 1){
		        					targetCellRow[i] = obj_list[j]->getRow();
		        					targetCellColumn[i] = obj_list[j]->getColumn();
		        					cout << world->getMyHeroes()[i]->getId() << "go to help" << targetCellRow[i] << ":" << targetCellColumn[i] << endl;
		        					break;
		        				}
		        			}
		        		}
		        	}
	        	}
	        }
	    }

    }
    turn = (turn + 1) % 6;
    

    vector<Hero *> my_heros = world->getMyHeroes();
    for(int i=0; i < 4; ++i){
        vector<Direction> _dirs = world->getPathMoveDirections(my_heros[i]->getCurrentCell().getRow(),
                                                               my_heros[i]->getCurrentCell().getColumn(),
                                                               targetCellRow[i],
                                                               targetCellColumn[i]);
        if(_dirs.size() == 0)//ALWAYS check if there is a path to that target!!!!
            continue;

        int status = checkMove(world, my_heros, i, _dirs[0]);
    	if(status == 1 && !my_heros[i]->getCurrentCell().isInObjectiveZone()){
    		// move different
    		for (int i = 0; i < 4; ++i)
    		{
    			Direction tmp[4] = {UP, DOWN, RIGHT, LEFT};
    			_dirs[0] = tmp[rand() % 4];
    			
    			if (!checkMove(world, my_heros, i, _dirs[0]))
                    break;
    		}
    	}
    	// اگه میتونن شلیک کنند AP رو خالی نکن
    	if(my_heros[i]->getCurrentCell().isInObjectiveZone() && world->getAP() < 50){

    	}else
        	world->moveHero(my_heros[i]->getId(), _dirs[0]);
    }
}


void AI::action(World *world) {
    cerr << "-action" << endl;

    for(Hero* my_hero : world->getMyHeroes()){
        if(my_hero->getName() == HeroName::BLASTER){

            // find the weakest that can attack with bomb
        	int bombingCooldown = my_hero->getAbility(AbilityName::BLASTER_BOMB).getRemCooldown();
			Cell bombing_cell = Cell::NULL_CELL;
			Cell closest_bombing = Cell::NULL_CELL;
            int min_dist_bomb = 10000;
            int hp_bomb = 1000;
            for(Hero* opp_hero : world->getOppHeroes()){
                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
                {
                    int distance = world->manhattanDistance(opp_hero->getCurrentCell(), my_hero->getCurrentCell());
                    if(distance <= 5){
                        min_dist_bomb = distance;
                        if(hp_bomb > opp_hero->getCurrentHP()){
                        	hp_bomb = opp_hero->getCurrentHP();
                        	bombing_cell = opp_hero->getCurrentCell();
                        }
                    }
                    if(distance < min_dist_bomb){
                    	closest_bombing = opp_hero->getCurrentCell();
                    	min_dist_bomb = distance;
                    }
                }
            }
            // find weakest that can attack
            Cell attack_cell = Cell::NULL_CELL;
            int min_dist_attack = 10000;
            int hp_attack = 1000;
            for(Hero* opp_hero : world->getOppHeroes()){
                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
                {
                    int distance = world->manhattanDistance(opp_hero->getCurrentCell(), my_hero->getCurrentCell());
                    if(distance <= 4){
                        min_dist_attack = distance;
                        if(hp_attack > opp_hero->getCurrentHP()){
                        	hp_attack = opp_hero->getCurrentHP();
                        	attack_cell = opp_hero->getCurrentCell();
                        }
                    }
                    if(distance < min_dist_attack)
                    	min_dist_attack = distance;
                }
            }

        	if(bombingCooldown <= 0 && min_dist_bomb <= 6 && world->getAP() >= 25){
        		// Do bombing
        		if(bombing_cell != Cell::NULL_CELL) {
                	world->castAbility(*my_hero, AbilityName::BLASTER_BOMB,bombing_cell);
            	}
        	}else if(min_dist_attack <= 4){
        		//Perform the bombing
            	if(attack_cell != Cell::NULL_CELL) {
                	world->castAbility(*my_hero, AbilityName::BLASTER_ATTACK,attack_cell);
            	}
        	}else{
        		// Do dodge
        		std::vector<Cell *> obj_list = world->map().getObjectiveZone();
        		int m = -1;
        		Cell dodgeCell = Cell::NULL_CELL;
        		for (int k = 0; k < obj_list.size(); ++k){
        			int distance = world->manhattanDistance(*obj_list[k], my_hero->getCurrentCell());
        			if(distance <= 4 && distance > m){
        				m = distance;
        				dodgeCell = *obj_list[k];
        			}
        		}
        		if(m != -1){
        			world->castAbility(*my_hero, AbilityName::BLASTER_DODGE,dodgeCell);
        		}else if(bombing_cell != Cell::NULL_CELL) {
                	//world->castAbility(*my_hero, AbilityName::BLASTER_BOMB,bombing_cell);
            	}
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
            int hp = 1000;
            for(Hero* _hero : world->getMyHeroes()){
                int distance = world->manhattanDistance(_hero->getCurrentCell(), my_hero->getCurrentCell());
                if(distance <= 4 && hp > _hero->getCurrentHP() &&
                        _hero->getRemRespawnTime() == 0 &&
                        _hero->getCurrentHP() != _hero->getMaxHP() && _hero->getName() != HeroName::HEALER){
                    min_dist = world->manhattanDistance(_hero->getCurrentCell(),
                                                        my_hero->getCurrentCell());
                	hp = _hero->getCurrentHP();
                    target_heal_cell = _hero->getCurrentCell();
                }
            }


            int healingCooldown = my_hero->getAbility(AbilityName::HEALER_HEAL).getRemCooldown();
            int heal = 0;
            if(healingCooldown <= 0){
            	if(min_dist > 4 || (my_hero->getCurrentCell().isInObjectiveZone() && my_hero->getCurrentHP() < 130)){
            		// heal himself
            		target_heal_cell = my_hero->getCurrentCell();
            		heal = 1;
            		//Do the heal
            		if(target_heal_cell != Cell::NULL_CELL) {
                		world->castAbility(*my_hero, AbilityName::HEALER_HEAL,target_heal_cell);
            		}
            	}else if(min_dist <= 4){
            		//Do the heal
            		if(target_heal_cell != Cell::NULL_CELL) {
                		world->castAbility(*my_hero, AbilityName::HEALER_HEAL,target_heal_cell);
            		}
            		heal = 1;		
            	}
            }
            if(heal == 0){
            	// do attack
            	//Find the closest bombing target
	            Cell bombing_cell = Cell::NULL_CELL;
	            int min_dist = 10000;
	            for(Hero* opp_hero : world->getOppHeroes()){
	                if(opp_hero->getCurrentCell().isInVision())//if hero is seen
	                {
	                    int distance = world->manhattanDistance(opp_hero->getCurrentCell(), my_hero->getCurrentCell());
	                    if(min_dist > distance){
	                        min_dist = world->manhattanDistance(opp_hero->getCurrentCell(),
	                                                            my_hero->getCurrentCell());
	                        bombing_cell = opp_hero->getCurrentCell();
	                    }
	                }
	            }
            	if(min_dist <= 4){
            		if(bombing_cell != Cell::NULL_CELL) {
                		world->castAbility(*my_hero, AbilityName::HEALER_ATTACK,bombing_cell);
            		}
            	}else{
            		// Do dodge
	        		std::vector<Cell *> obj_list = world->map().getObjectiveZone();
	        		int m = -1;
	        		Cell dodgeCell = Cell::NULL_CELL;
	        		for (int k = 0; k < obj_list.size(); ++k){
	        			int distance = world->manhattanDistance(*obj_list[k], my_hero->getCurrentCell());
	        			if(distance <= 4 && distance > m){
	        				m = distance;
	        				dodgeCell = *obj_list[k];
	        			}
	        		}
	        		if(m != -1){
        				world->castAbility(*my_hero, AbilityName::HEALER_DODGE,dodgeCell);
        			}else if(target_heal_cell != Cell::NULL_CELL) {
                		world->castAbility(*my_hero, AbilityName::HEALER_ATTACK,target_heal_cell);
            		}
            	}	
            }
            
            
        }
    }


    printMap(world);
}