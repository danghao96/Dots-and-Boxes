/*
Hao Dang danghao@xserver
2016-04-06
honors2.cpp
*/

#include <iostream>
using std::cout; using std::endl; using std::cin;
#include <sstream>
using std::ostringstream;
#include <fstream>
using std::ofstream;
#include <iomanip>
using std::setw; using std::boolalpha; using std::setprecision;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <utility>
using std::pair;
#include <map>
using std::map;
#include <algorithm>
using std::sort;
#include <random>
using std::mt19937_64; using std::uniform_int_distribution;

struct Box{
	//This represent single box in a game board.
	int x_pos, y_pos;
	int value;					//0:none; 1:A; 2:B;
	int sides;					//the number of sides that box have.
	bool left, right, up, down;
	Box(int x, int y) : x_pos(x), y_pos(y), value(0), sides(0), 
	left(false), right(false), up(false), down(false) {};
};

class Game{
	//This represent a complete game. Include a board, and keep tracking
	//scores.
  private:
	size_t row_ = 3;
	size_t col_ = 3;
	int remain_ = row_ * (col_ + 1) + (row_ + 1) * col_;
	
	vector<vector<Box>> board;
	int round_ = 0;	//determine who's turn
	vector<pair<int, int>> available_move_;
	
	int score_a_ = 0;
	int score_b_ = 0;
	int score_sum_ = score_a_ + score_b_;
	
  public:
	Game(int start_round = 0);
	void reset();
	void sync();
	
	bool check_for_box(int player);
	bool check_move(pair<int, int> m);
	void move(pair<int, int> movement);
	
	void winning_play(int player, bool if_draw, ostringstream& result);
	void random_play(int player, bool if_draw, ostringstream& result);
	void play(ofstream& ofile, bool if_draw);
	
	void draw_board(int player, pair<long, long> move, ostringstream& result);
};

//Global Variables.
mt19937_64 rand_eng(0);

//Total wins
long win_a = 0;
long win_b = 0;
//Total scores. The sum of scores of each single game.
long score_a_total = 0;
long score_b_total = 0;
//Single scores stored in vector.
vector<int> score_a_vector;
vector<int> score_b_vector;
vector<int> score_vector;

Game::Game(int start_round){
	row_ = 3;
	col_ = 3;
	round_ = start_round;
	
	//Build the board.
	for(size_t y = 0; y < row_; y++){
		vector<Box> temp_row;
		for(size_t x = 0; x < col_; x++){
			Box temp(x, y);
			temp_row.push_back(temp);
		}
		board.push_back(temp_row);
	}
	
	//Build available_move_
	for(size_t i = 0; i < (row_ * (col_ + 1)); i++){
		int first = i, second = first + col_ + 1;
		available_move_.push_back({first, second});
	}
	for(size_t i = 0; i <= row_; i++){
		for(size_t j = 0; j < col_; j++){
			int first = (col_ + 1) * i + j, second = first + 1;
			available_move_.push_back({first, second});
		}
	}
}

/*--------------------------------------------------------------------*/
const string hor_line("  - ");
const string hor_noline("    ");
const string ver_line(" |");
const string ver_noline("  ");
const string mark_a("  A ");
const string mark_b("  B ");
const string mark_0("    ");
const int setwvalue = 2;

void draw_horizontal(vector<vector<Box>> boxes, ostringstream &oss, 
int row_num, int row, int col){
	//This function will draw horizontal line and dots. This is serve
	//for single_print function.
	int dot = row_num * (col + 1);	//get the value of first dot to draw
	if(row_num < row){
		//if the line to draw is not the last line.
		for(int i = 0; i < col; i++){
			oss << setw(setwvalue) << dot + i;
			//get the box.up value to draw.
			if(boxes[row_num][i].up){ oss << hor_line; }
			else{ oss << hor_noline; }
		}
	}else if(row_num == row){
		//if the line to draw is the last line.
		for(int i = 0; i < col; i++){
			oss << setw(setwvalue) << dot + i;
			//get the previous line's box.down value to draw.
			if(boxes[row_num - 1][i].down){	oss << hor_line; }
			else{ oss << hor_noline; }
			
		}
	}
	//draw the last dot of each row;
	oss << setw(setwvalue) << dot + col << endl;
}

void draw_vertical(vector<vector<Box>> boxes, ostringstream &oss, 
int row_num, int row, int col){
	//This function will draw vertical line and box content. This is
	//serve for single_print function.
	if(row_num < row){
		//the value of row_num must smaller than the value of row.
		for(int i = 0; i < col; i++){
			//loop to draw for all colonm.
			
			//draw the left line of each box.
			if(boxes[row_num][i].left){ oss << ver_line; }
			else{ oss << ver_noline; }
			
			//draw the value of each box.
			if(boxes[row_num][i].value == 1){ oss << mark_a; }
			else if(boxes[row_num][i].value == 2){ oss << mark_b; }
			else{ oss << mark_0; }
		}
		
		//draw the last line of each row.
		if(boxes[row_num][col-1].right){ oss << ver_line << endl;	}
		else{ oss << ver_noline << endl; }
	}
}

void Game::draw_board(int player_int, pair<long, long> move, ostringstream& result){
	//This function print out the information of single play.
	char player = player_int + 64;
	//Draw Board
	for(size_t i = 0; i <= row_; i++){
		ostringstream oss;
		draw_horizontal(board, oss, i, row_, col_);
		result << oss.str();
		oss.str("");
		draw_vertical(board, oss, i, row_, col_);
		result << oss.str();
	}
	
	//Print out information
	result << "Score is A:" << score_a_ << " and B:" << score_b_ << endl;
	result << "This is " << player << "'s move." << endl;
	result << "The move is: " << move.first << ", " << move.second << endl;
	
	int box_made = score_a_ + score_b_ - score_sum_;
	switch( box_made ){
		case 0:
			result << "No box was made." << endl;
			break;
		case 1:
			result << "1 box was made." << endl;
			break;
		default:
			result << box_made << " boxes were made." << endl;
	}
	if(remain_){
		if(box_made){
			result << player << " will go again." << endl;
		}else{
			result << player << " will not go again." << endl;
		}			
	}
	score_sum_ = score_a_ + score_b_;
	result << endl;
}

void Game::sync(){
	//This function is used for sync the boxes. For example, when the
	//line of boxes[0][1].down drawn, the line of boxes[1][1].up should
	//also be drawn. This function is used for that situation.
	for(size_t i = 0; i < row_; i++){
		//loop througn every box and test.
		for(size_t j = 0; j < col_ - 1; j++){
			//sync the vertical line of each box.
			if(board[i][j].right^board[i][j+1].left){
				board[i][j].right = board[i][j+1].left = 1;
			}
		}
		if(i < row_ - 1){
			for(size_t j = 0; j < col_; j++){
				//sync the horizental line of each box.
				if(board[i][j].down^board[i+1][j].up){
					board[i][j].down = board[i+1][j].up = 1;
				}
			}
		}
	}
	
	//calculate the sides_ value for each box.
	for(size_t i = 0; i < row_; i++){
		for(size_t j = 0; j < col_; j++){
			int temp = 0;
			if(board[i][j].left){ temp++; }
			if(board[i][j].right){ temp++; }
			if(board[i][j].up){ temp++; }
			if(board[i][j].down){ temp++; }
			board[i][j].sides = temp;
		}
	}
}

bool Game::check_for_box(int player){
    //This function checks if a movement result a box. If so, set the 
    //value of the box, and return true. Otherwise, return false.
	bool result = false;
    for(size_t y = 0; y < row_; y++){
        for(size_t x = 0; x < col_; x++){
			//loop through every box and test them.
            if(!board[y][x].value){
				//Only if the value of the box is not set yet, test that
				//box. otherwise, ignore that box.
                if(board[y][x].left && board[y][x].right && board[y][x].up 
                && board[y][x].down){
					//if the box has four edges, set its value.
                    board[y][x].value = player;
                    if(player == 1){ score_a_++; result = true;}
                    else if(player == 2){ score_b_++; result = true;}
                }
            }
        }
    }
    return result;
}

void Game::move(pair<int, int> movement){
	//This function will make a move depends on the input pair.
	size_t x, y;
	x = movement.first % (col_+1);
	y = movement.first / (col_+1);
	size_t difference = abs(movement.first - movement.second);
	if(difference == 1){
		//if the move is a horizental line, which means the difference
		//between two integer is equal to 1, set the up or down value.
		if(y < row_){
			board[y][x].up = 1;
		}else{
			board[y-1][x].down = 1;
		}
	}else if(difference == col_+1){
		//if the move is a vertical line, which means the difference
		//between two integer is equal to 4, set the left or right value.
		if(x < col_){
			board[y][x].left = 1;
		}else{
			board[y][x-1].right = 1;
		}
	}
	sync();			//call sync function to sync the boxes.
	remain_--;		//after the move, remain_ minus 1.
}

void Game::winning_play(int player, bool if_draw, ostringstream& result){
	//This is winning player, it will try to win the game.
	pair<int, int> movement = {0, 0};
	bool get_move = false;
	
	//First Strategy: Add the forth edge.
	if(!get_move){
		for(size_t y = 0; y < row_; y++){
			for(size_t x = 0; x < col_; x++){
				if(board[y][x].left && board[y][x].right 
				&& board[y][x].up && (!(board[y][x].down))){
					movement = {(y+1)*(col_+1)+x, (y+1)*(col_+1)+x+1};
					get_move = true;
				}else if(board[y][x].left && board[y][x].right 
				&& (!(board[y][x].up)) && board[y][x].down){
					movement = {y*(col_+1)+x, y*(col_+1)+x+1};
					get_move = true;
				}else if(board[y][x].left && (!(board[y][x].right)) 
				&& board[y][x].up && board[y][x].down){
					movement = {y*(col_+1)+x+1, y*(col_+1)+x+1+(col_+1)};
					get_move = true;
				}else if(((!(board[y][x].left)) && board[y][x].right 
				&& board[y][x].up && board[y][x].down)){
					movement = {y*(col_+1)+x, y*(col_+1)+x+(col_+1)};
					get_move = true;
				}
			}
		}
	}
	
	//Second Strategy: Do not add the third edge.
	if(!get_move){
		vector<pair<int, int>> untested_move = available_move_;
		while(untested_move.size()){
			uniform_int_distribution<long> dist(0, untested_move.size() - 1);
			long rand = dist(rand_eng);
			pair<int, int> test_move = untested_move[rand];

			size_t x, y;
			x = test_move.first % (col_+1);
			y = test_move.first / (col_+1);
			size_t difference = abs(test_move.first - test_move.second);
			if(difference == 1){
				if((y == 0) && (board[y][x].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}else if((y > 0 && y < row_) && (board[y-1][x].sides < 2) 
				&& (board[y][x].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}else if((y == row_) && (board[y - 1][x].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}
			}else if(difference == col_+1){
				if((x == 0) && (board[y][x].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}else if((x > 0 && x < col_) && (board[y][x-1].sides < 2) 
				&& (board[y][x].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}else if((x == col_) && (board[y][x-1].sides < 2)){
					movement = test_move;
					get_move = true;
					break;
				}
			}
			untested_move.erase(untested_move.begin() + rand);
		}
	}
	
	//Last Strategy: Randomly pick a move.
	if(!get_move){
		uniform_int_distribution<long> dist(0, remain_ - 1);
		long rand = dist(rand_eng);
		movement = available_move_[rand];
	}
	
	//get the index of movement, thus it can be erased.
	size_t index;
	for(index = 0; index < available_move_.size(); index++){
		if(available_move_[index] == movement){
			break;
		}
	}
	
	move(movement);	//make the move
	round_ += 1;	//go to next round
	//if boxes are made, will not change player.
	if(check_for_box(player)){ round_ += 1; }
	
	available_move_.erase(available_move_.begin()+index);
	if(if_draw){ draw_board(player, movement, result); }
}

void Game::random_play(int player, bool if_draw, ostringstream& result){
	//This is random player. it will ramdomly pick a move from 
	//available_move_ vector.
	//Pick the move
	uniform_int_distribution<long> dist(0, remain_ - 1);
	long rand = dist(rand_eng);
	pair<int, int> movement = available_move_[rand];
	
	move(movement);	//make the move
	round_ += 1;	//go to next round
	//if boxes are made, will not change player.
	if(check_for_box(player)){ round_ += 1; }
	
	available_move_.erase(available_move_.begin()+rand);
	if(if_draw){ draw_board(player, movement, result); }
}

void Game::play(ofstream& ofile, bool if_draw){
	//play the game and update score informations.
	ostringstream oss;
	while(remain_){
		//loop to finish the game.
		if((round_ % 2) == 0){
			random_play(1, if_draw, oss);
		}else{
			winning_play(2, if_draw, oss);
		}
	}
	
	//update scores.
	score_a_total += score_a_;
	score_b_total += score_b_;
	score_a_vector.push_back(score_a_);
	score_b_vector.push_back(score_b_);
	score_vector.push_back(score_a_);
	score_vector.push_back(score_b_);
	
	//output the game information.
	if(if_draw){
		ofile << oss.str() << endl;
	}
	if(score_a_ > score_b_){
		win_a++;
	}else{
		win_b++;
	}
}

int main(){
	long random_seed = 0;
	cout << "Enter a random seed: "; cin >> random_seed;
	mt19937_64 reng(random_seed);
	rand_eng = reng;
	long rounds_to_play = 1000;
	cout << "Enter the number of rounds to play: "; cin >> rounds_to_play;
	
	//Single Play
	ofstream sfile("single_play.txt");
	sfile << "Random Player First" << endl;
	Game single_random_first(0);
	single_random_first.play(sfile, true);
	sfile << "Winning Player First" << endl;
	Game single_winning_first(1);
	single_winning_first.play(sfile, true);
	sfile.close();
	
	//Multiple Play
	win_a = 0; win_b = 0; score_a_total = 0; score_b_total = 0;
	score_a_vector.clear(); score_b_vector.clear(); score_vector.clear();
	
	ofstream mfile("multiple_play.txt");
	uniform_int_distribution<int> dist(0,1);
	for(int i = 0; i < rounds_to_play; i++){
		int start_round = dist(rand_eng);
		Game multiple(start_round);
		multiple.play(mfile, false);
	}
	//Winning play statistic information output.
	double score_ave = (score_a_total + score_b_total) / (2.0 * rounds_to_play);
	double score_a_ave = score_a_total / (1.0 * rounds_to_play);
	double score_b_ave = score_b_total / (1.0 * rounds_to_play);
	sort(score_a_vector.begin(), score_a_vector.end());
	sort(score_b_vector.begin(), score_b_vector.end());
	sort(score_vector.begin(), score_vector.end());
	int score_med = score_vector[rounds_to_play];
	int score_a_med = score_a_vector[rounds_to_play/2];
	int score_b_med = score_b_vector[rounds_to_play/2];
	int score_a_max = score_a_vector.back();
	int score_a_min = score_a_vector.front();
	int score_b_max = score_b_vector.back();
	int score_b_min = score_b_vector.front();
	double win_rate = win_b / (1.0 * rounds_to_play);
	
	mfile << "Multiple Play Report" << endl;
	mfile << "Player A is Random Player. Player B is Winning Player." << endl;
	mfile << "Total number of rounds played: " << rounds_to_play << endl;
	mfile << "Overall average score: " << score_ave << endl;
	mfile << "A's average score: " << score_a_ave << endl;
	mfile << "B's average score: " << score_b_ave << endl;
	mfile << "Overall median score: " << score_med << endl;
	mfile << "A's median score: " << score_a_med << endl;
	mfile << "B's median score: " << score_b_med << endl;
	mfile << "A's highest score: " << score_a_max << endl;
	mfile << "A's lowest score: " << score_a_min << endl;
	mfile << "B's highest score: " << score_b_max << endl;
	mfile << "B's lowest score: " << score_b_min << endl;
	mfile << "Total number of round A wins: " << win_a << endl;
	mfile << "Total number of round A wins: " << win_b << endl;
	mfile << "Winning Player winning rate: " << win_rate << endl;
	mfile.close();
}
