#include <chrono>
#include <random>
#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>

#include "Headers/DrawText.hpp"
#include "Headers/Global.hpp"
#include "Headers/GetTetromino.hpp"
#include "Headers/GetWallKickData.hpp"
#include "Headers/Tetromino.hpp"




int main()
{
	//Used to check whether the game is over or not
	bool game_over = 0;
	//Is the hard drop button pressed?
	bool hard_drop_pressed = 0;
	//Is the rotate button pressed?
	bool rotate_pressed = 0;
	//Is the paused button pressed?
	bool pause_pressed = 0;
	//Is the bomb exploded?
	bool explode = 0;
	//Is the button pressed?
	bool button_pressed = 0;
	//Is the music playing?
	bool musicPlaying = 0;
	//IS music button pressed?
	bool music_pressed = 0;
	//Is clear button pressed?
	bool clear_pressed = 0;
	//Is color button pressed?
	bool color_pressed = 0;


	//Used to make the game framerate-independent
	unsigned lag = 0;
	//How many lines the player cleared?
	unsigned lines_cleared = 0;
	//scores
	unsigned explode_scores = 0;
	unsigned scores = 0;
	//Used to control yhe ratio of falling speed
	unsigned ratio = 1;
	//Used to define color
	unsigned color = 0;

	//Timer for the line clearing effect
	unsigned char clear_effect_timer = 0;
	//Gee, I wonder what this is
	unsigned char current_fall_speed = START_FALL_SPEED;
	//Timer for the tetromino falling
	unsigned char fall_timer = 0;
	//Timer for the tetromino moving horizontally
	unsigned char move_timer = 0;
	//Next shape (The shape that comes after the current shape)
	unsigned char next_shape;
	//Timer for the tetromino's soft drop
	unsigned char soft_drop_timer = 0;

	//
	unsigned char row;
	unsigned char column;

	//Similar to lag, used to make the game framerate-independent
	std::chrono::time_point<std::chrono::steady_clock> previous_time;

	//I don't really know what this does, so I'm gonna assume this is a random device
	std::random_device random_device;

	//Random engine
	std::default_random_engine random_engine(random_device());

	//I KNOW THIS ONE! Distribution of all the shapes. We're gonna randomly choose one from them
	std::uniform_int_distribution<unsigned short> shape_distribution(0, 7);

	//Stores the current state of each row. Whether they need to be cleared or not
	std::vector<bool> clear_lines(ROWS, 0);

	//All the colors for the cells
	std::vector<sf::Color> cell_colors = {
		sf::Color(36, 36, 85),
		sf::Color(0, 219, 255),
		sf::Color(0, 36, 255),
		sf::Color(255, 146, 0),
		sf::Color(255, 219, 0),
		sf::Color(0, 219, 0),
		sf::Color(146, 0, 255),
		sf::Color(219, 0, 0),
		sf::Color(219,0,0),
		sf::Color(73, 73, 85)
	};
	std::vector<sf::Color>bachground_colors = {
		 sf::Color(255, 192, 203), // 粉色
		 sf::Color(173, 216, 230), // 浅紫色
		 sf::Color(144, 238, 144),// 浅绿色
		 sf::Color(255, 192, 203)
	};

	//Game matrix. Everything will happen to this matrix
	std::vector<std::vector<unsigned char>> matrix(COLUMNS, std::vector<unsigned char>(ROWS));

	//SFML thing. Stores events, I think
	sf::Event event;

	//Window
	sf::RenderWindow window(sf::VideoMode(2 * CELL_SIZE * COLUMNS * SCREEN_RESIZE, CELL_SIZE * ROWS * SCREEN_RESIZE), "Tetris", sf::Style::Close);
	//Resizing the window
	window.setView(sf::View(sf::FloatRect(0, 0, 2 * CELL_SIZE * COLUMNS, CELL_SIZE * ROWS)));
	

	//Falling tetromino. At the start we're gonna give it a random shape
	Tetromino tetromino(static_cast<unsigned char>(shape_distribution(random_engine)), matrix);

	//Generate a random shape and store it as the next shape
	next_shape = static_cast<unsigned char>(shape_distribution(random_engine));

	//Get the current time and store it in the variable
	previous_time = std::chrono::steady_clock::now();
	// Load and play music
	sf::Music music;
	if (!music.openFromFile("Resources/Sound/backgroundMusic.ogg")) {
		return -1;
	}
	music.setLoop(true);
	music.play();
	musicPlaying = true;

	//While the window is open
	while (1 == window.isOpen())
	{
	
		//Draw the menubutton
		sf::RectangleShape menuButton(sf::Vector2f(10, 10)); // 尺寸和位置
		sf::Texture menubuttonTexture;
		menubuttonTexture.loadFromFile("Resources/Images/menubutton.png");
		menuButton.setTexture(&menubuttonTexture); // 设置纹理
		menuButton.setPosition(140, 5); // 放在屏幕的右上角
		window.draw(menuButton);
		//Get the difference in time between the current frame and the previous frame
		unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();

		//Add the difference to the lag
		lag += delta_time;

		//In other words, we're updating the current time for the next frame.
		previous_time += std::chrono::microseconds(delta_time);

		//While the lag exceeds the maximum allowed frame duration
		while (FRAME_DURATION <= lag)
		{
			//Subtract the right thing from the left thing
			lag -= FRAME_DURATION;

			//Looping through the events
			while (1 == window.pollEvent(event))
			{
				//Check the event type
				switch (event.type)
				{
					//If the user closed the game
					case sf::Event::Closed:
					{
						//Close the window
						window.close();

						break;
					}
					case sf::Event::MouseButtonPressed:
					{
						if (event.mouseButton.button == sf::Mouse::Left) 
						{
							// 获取鼠标点击位置
							sf::Vector2i mousePos = sf::Mouse::getPosition(window);

							// 转换为世界坐标
							sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

							// 检查鼠标点击是否在menuButton的范围内
							if (menuButton.getGlobalBounds().contains(worldPos))
							{
								button_pressed = 1;
							}
						}
				
						break;
					}
					//If the key has been released
					case sf::Event::KeyReleased:
					{
						//Check which key was released
						switch (event.key.code)
						{
							//If it's C or Z
							case sf::Keyboard::C:
							case sf::Keyboard::Z:
							{
								//Rotation key is not pressed anymore
								rotate_pressed = 0;

								break;
							}
							//If it's Down
							case sf::Keyboard::Down:
							{
								//Reset the soft drop timer
								soft_drop_timer = 0;

								break;
							}
							//If it's Left or Right
							case sf::Keyboard::Left:
							case sf::Keyboard::Right:
							{
								//Reset the move timer
								move_timer = 0;

								break;
							}
							//If it's Space
							case sf::Keyboard::Space:
							{
								//Hard drop key is not pressed anymore
								hard_drop_pressed = 0;

								break;
							}
							//If it's NUM1
							case sf::Keyboard::Num1:
							{
								ratio = 1;
								break;
							}
							//If it"s NUm2
							case sf::Keyboard::Num2:
							{
								ratio = 2;

								break;
							}
							case sf::Keyboard::Num4:
							{
								ratio = 4;

								break;
							}
							case sf::Keyboard::Escape:
							{
								pause_pressed = !pause_pressed;
							}
						}
					}
				}
			}

			current_fall_speed = START_FALL_SPEED / ratio;
			if (button_pressed == 1)
			{

				sf::RenderWindow newWindow(sf::VideoMode(400, 300), "New Window");

				// 设置为false以避免不断创建新窗口
                button_pressed = false;

				// 新窗口事件处理循环
				while (newWindow.isOpen())
				{
					// 创建一个矩形按钮
					sf::RectangleShape musicbutton(sf::Vector2f(50, 50));
					musicbutton.setPosition(140, 50); // 设置按钮位置
					sf::Texture musicdisplayTexture;
					musicdisplayTexture.loadFromFile("Resources/Images/musicdisplay.png");
					sf::Texture musicplayTexture;
					musicplayTexture.loadFromFile("Resources/Images/musicplay.png");

					sf::RectangleShape clearbutton(sf::Vector2f(100, 50));
					clearbutton.setPosition(140, 150); // 设置按钮位置
					sf::Texture clearTexture;
					clearTexture.loadFromFile("Resources/Images/clear.png");
					clearbutton.setTexture(&clearTexture);

					sf::RectangleShape colorbutton(sf::Vector2f(100, 50));
					colorbutton.setPosition(200, 50); // 设置按钮位置
					sf::Texture colorTexture;
					colorTexture.loadFromFile("Resources/Images/color.png");
					colorbutton.setTexture(&colorTexture);
					if (musicPlaying)
					{
						musicbutton.setTexture(&musicplayTexture); // 设置纹理 
					}
					else
					{
						musicbutton.setTexture(&musicdisplayTexture); // 设置纹理 
					}
					draw_text(140, 50, "Music" , newWindow);
					sf::Event newEvent;
					while (newWindow.pollEvent(newEvent))
					{
						if (newEvent.type == sf::Event::Closed)
						{
							newWindow.close();
						}
						else if (newEvent.type == sf::Event::MouseButtonPressed)
						{
							if (newEvent.mouseButton.button == sf::Mouse::Left)
							{
								// 获取鼠标点击位置
								sf::Vector2i mousePos = sf::Mouse::getPosition(newWindow);

								// 转换为世界坐标
								sf::Vector2f worldPos = newWindow.mapPixelToCoords(mousePos);

								// 检查鼠标点击是否在menuButton的范围内
								if (musicbutton.getGlobalBounds().contains(worldPos))
								{
									music_pressed = 1;
								}
								else if (clearbutton.getGlobalBounds().contains(worldPos))
								{
									clear_pressed = 1;
								}
								else if (colorbutton.getGlobalBounds().contains(worldPos))
								{
									color_pressed = 1;
								}
							}
						}
					}
				
				   // 如果按钮被按下，开关音乐
					if (music_pressed)
					{
						if (musicPlaying)
						{
							music.stop();

							musicbutton.setTexture(&musicdisplayTexture); // 设置纹理 
						}
						else
						{
							music.play();
							
							musicbutton.setTexture(&musicplayTexture); // 设置纹理 
						}
							musicPlaying = !musicPlaying;
							music_pressed = 0; // 重置按钮状态
					}
					
					newWindow.clear();
					// 在这里添加绘图代码，如果有的话
					newWindow.draw(musicbutton);
					newWindow.draw(clearbutton);
					newWindow.draw(colorbutton);

					newWindow.display();
				}

			}
			if (pause_pressed)
			{
				continue;
			}
			if (clear_pressed == 1)
			{
				for (unsigned char a = 0; a < ROWS; a++)
				{
					for (unsigned char b = 0; b < COLUMNS; b++)
					{
						matrix[b][a] = 0;
					}
				}
				clear_pressed = 0;
			}
			//If the clear effect timer is 0
			if (0 == clear_effect_timer)
			{
				//If the game over is 0
				if (0 == game_over)
				{
					//If the rotate pressed is 0 (Damn, I'm so good at commenting!)
					if (0 == rotate_pressed)
					{
						//If the C is pressed
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::C))
						{
							//Rotation key is pressed!
							rotate_pressed = 1;

							//Do a barrel roll
							tetromino.rotate(1, matrix);
						} //Else, if the Z is pressed
						else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
						{
							//Rotation key is pressed!
							rotate_pressed = 1;

							//Do a barrel roll but to the other side!
							tetromino.rotate(0, matrix);
						}
					}

					//If the move timer is 0
					if (0 == move_timer)
					{
						//If the Left is pressed
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
						{
							//Reset the move timer
							move_timer = 1;

							//Move the tetromino to the left
							tetromino.move_left(matrix);
						}
						else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
						{
							//Reset the move timer
							move_timer = 1;

							//Move the tetromino to the opposite of left
							tetromino.move_right(matrix);
						}
					}
					else
					{
						//Update the move timer
						move_timer = (1 + move_timer) % MOVE_SPEED;
					}

					//If hard drop is not pressed
					if (0 == hard_drop_pressed)
					{
						//But the Space is pressed, which is the hard drop key (Paradox?)
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
						{
							//Get rid of the paradox!
							hard_drop_pressed = 1;

							//Reset the fall timer
							fall_timer = current_fall_speed;

							//Make the falling tetromino drop HAAAAARD!
							tetromino.hard_drop(matrix);
						}
					}

					//I don't wanna rewrite the same thing again so just look at the comments above and use your head
					if (0 == soft_drop_timer)
					{
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
						{
							if (1 == tetromino.move_down(matrix))
							{
								fall_timer = 0;
								soft_drop_timer = 1;
							}
						}
					}
					else
					{
						soft_drop_timer = (1 + soft_drop_timer) % SOFT_DROP_SPEED;
					}

					//If the fall timer is over
					if (current_fall_speed == fall_timer)
					{
						//If the tetromino can't move down anymore
						if (0 == tetromino.move_down(matrix))
						{
							//Put the falling tetromino to the matrix
							tetromino.update_matrix(matrix);
							//Explode!
							for (unsigned char a = 0; a < ROWS; a++)
							{
								for (unsigned char b = 0; b < COLUMNS; b++)
								{
									if (matrix[b][a] == 8)
									{
										explode = 1;
										row = a;
										column = b;
									}
								}
							}
							if (explode == 1)
							{
								clear_effect_timer = CLEAR_EFFECT_DURATION;
							}
					
					
							//Loop through every row
							for (unsigned char a = 0; a < ROWS; a++)
							{
								//Here we're gonna check if the current row should be cleared or not
								bool clear_line = 1;

								//Check if the every cell in the row is filled or not
								for (unsigned char b = 0; b < COLUMNS; b++)
								{
									if (0 == matrix[b][a])
									{
										clear_line = 0;
										break;
									}
							
								}

								//If we have to clear it
								if (1 == clear_line)
								{
									//WE CLEAR IT!
									//First we increase the score
									lines_cleared++;

									//Then we start the effect timer
									clear_effect_timer = CLEAR_EFFECT_DURATION;

									//Set the current row as the row that should be cleared
									clear_lines[a] = 1;

									//If the player reached a certain number of lines
									if (0 == lines_cleared % LINES_TO_INCREASE_SPEED)
									{
										//We increase the game speed
										current_fall_speed = std::max<unsigned char>(SOFT_DROP_SPEED, current_fall_speed - 1);
									}
								}
							}

							//If the effect timer is over
							if (0 == clear_effect_timer)
							{
								//Decide if the game is over or not based on the return value of the reset function
								//Yes I know I could use "!" but it looks ugly and I hate using it!
								game_over = 0 == tetromino.reset(next_shape, matrix);

								//Generate the next shape
								next_shape = static_cast<unsigned char>(shape_distribution(random_engine));
							}
						}

						//Restart the fall timer
						fall_timer = 0;
					}
					else
					{
						//Increment the fall timer
						fall_timer++;
					}
				} //This is the code for restarting the game
				else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
				{
					//We set everything to 0
					game_over = 0;
					hard_drop_pressed = 0;
					rotate_pressed = 0;
					explode_scores = 0;
					lines_cleared = 0;

					//Except the current fall speed because he's a special boy
					current_fall_speed = START_FALL_SPEED/ratio;
					fall_timer = 0;
					move_timer = 0;
					soft_drop_timer = 0;

					//Then we clear the matrix
					for (std::vector<unsigned char>& a : matrix)
					{
						std::fill(a.begin(), a.end(), 0);
					}
				}
			}
			else
			{
				//Decrement the effect timer
				clear_effect_timer--;

				//If the effect timer is between 1 and -1
				if (0 == clear_effect_timer)
				{
					//Loop through each row
					for (unsigned char a = 0; a < ROWS; a++)
					{
						//If the row should be cleared
						if (1 == clear_lines[a])
						{
							//Loop through each cell in the row
							for (unsigned char b = 0; b < COLUMNS; b++)
							{
								//Set the cell to 0 (empty) (the absence of existence)
								matrix[b][a] = 0;

								//Swap the row with the rows above
								for (unsigned char c = a; 0 < c; c--)
								{
									matrix[b][c] = matrix[b][c - 1];
									matrix[b][c - 1] = 0;
								}
							}
						}
					
					}
					if (explode == 1)
					{
						for (unsigned char a = std::max(row - 1, 0); a <= std::min(row + 1, ROWS - 1); a++)
						{
							for (unsigned char b = std::max(column - 1, 0); b <= std::min(column + 1, COLUMNS - 1); b++)
							{
								if (matrix[b][a] > 0)
								{
									explode_scores++;
								}
								matrix[b][a] = 0;
							}

						}
					}
					explode = 0;
					//I already explained this earlier and I don't wanna do it again
					game_over = 0 == tetromino.reset(next_shape, matrix);

					next_shape = static_cast<unsigned char>(shape_distribution(random_engine));

					//Clear the clear lines array
					std::fill(clear_lines.begin(), clear_lines.end(), 0);
				}
			}

			
			
			
			
			//Here we're drawing everything!画图
			if (FRAME_DURATION > lag)
			{
				//Calculating the size of the effect squares
				unsigned char clear_cell_size = static_cast<unsigned char>(2 * round(0.5f * CELL_SIZE * (clear_effect_timer / static_cast<float>(CLEAR_EFFECT_DURATION))));

				//We're gonna use this object to draw every cell in the game
				sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
				//Next shape preview border (White square at the corner)
				sf::RectangleShape preview_border(sf::Vector2f(5 * CELL_SIZE, 5 * CELL_SIZE));
				preview_border.setFillColor(sf::Color(0, 0, 0));
				preview_border.setOutlineThickness(-1);
				preview_border.setPosition(CELL_SIZE * (1.5f * COLUMNS - 2.5f), CELL_SIZE * (0.25f * ROWS - 2.5f));

				//Clear the window from the previous frame
				if (color_pressed)
				{
					color++;
					color_pressed = 0;
				}
				window.clear(bachground_colors[color]);
				if (color == 3)
				{
					color = 0;
				}

				//Draw the matrix
				for (unsigned char a = 0; a < COLUMNS; a++)
				{
					for (unsigned char b = 0; b < ROWS; b++)
					{
						if (0 == clear_lines[b])
						{
							cell.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));

							if (1 == game_over && 0 < matrix[a][b])
							{
								cell.setFillColor(cell_colors[9]);
							}
							else
							{
								cell.setFillColor(cell_colors[matrix[a][b]]);
							}

							window.draw(cell);
						}
					}
				}

				//Set the cell color to gray
				cell.setFillColor(cell_colors[9]);

				//If the game is not over
				if (0 == game_over)
				{
					//Drawing the ghost tetromino
					for (Position& mino : tetromino.get_ghost_minos(matrix))
					{
						cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));

						window.draw(cell);
					}

					cell.setFillColor(cell_colors[1 + tetromino.get_shape()]);
				}

				//Drawing the falling tetromino
				
				std::vector<Position> minos1 = tetromino.get_minos();
				if (minos1[1].x==minos1[0].x && minos1[1].y == minos1[0].y)
				{
						sf::Texture bombTexture;
						bombTexture.loadFromFile("Resources/Images/Bomb.png");
						cell.setTexture(&bombTexture);
						cell.setPosition(static_cast<float>(CELL_SIZE* minos1[0].x), static_cast<float>(CELL_SIZE* minos1[0].y));
						window.draw(cell);
				}
				else
				{
					for (Position& mino : tetromino.get_minos())
					{
						cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));

						window.draw(cell);
					}
				}

				//Drawing the effect
				
				for (unsigned char a = 0; a < COLUMNS; a++)
				{
					for (unsigned char b = 0; b < ROWS; b++)
					{
						if (1 == clear_lines[b])
						{
							cell.setFillColor(cell_colors[0]);
							cell.setPosition(static_cast<float>(CELL_SIZE* a), static_cast<float>(CELL_SIZE* b));
							cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

							window.draw(cell);

							cell.setFillColor(sf::Color(255, 255, 255));
							cell.setPosition(floor(CELL_SIZE* (0.5f + a) - 0.5f * clear_cell_size), floor(CELL_SIZE* (0.5f + b) - 0.5f * clear_cell_size));
							cell.setSize(sf::Vector2f(clear_cell_size, clear_cell_size));

							window.draw(cell);
						}
					}
				}

				//Fanuitemg iow gfnreuignoei gnrign yerashr trujngfipoag
				cell.setFillColor(cell_colors[1 + next_shape]);
				cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

				//Draw the preview border
				window.draw(preview_border);

				//Draw the next tetromino
				for (Position& mino : get_tetromino(next_shape, static_cast<unsigned char>(1.5f * COLUMNS), static_cast<unsigned char>(0.25f * ROWS)))
				{
					//Shifting the tetromino to the center of the preview border
					unsigned short next_tetromino_x = CELL_SIZE * mino.x;
					unsigned short next_tetromino_y = CELL_SIZE * mino.y;

					if (0 == next_shape)
					{
						next_tetromino_y += static_cast<unsigned char>(round(0.5f * CELL_SIZE));
					}
					else if (3 != next_shape)
					{
						next_tetromino_x -= static_cast<unsigned char>(round(0.5f * CELL_SIZE));
					}

					cell.setPosition(next_tetromino_x, next_tetromino_y);
					if (next_shape == 7)
					{
						sf::Texture bombTexture;
						bombTexture.loadFromFile("Resources/Images/Bomb.png");
						cell.setTexture(&bombTexture);
					}

					window.draw(cell);
				}
				//Draw the menubutton
				sf::RectangleShape menuButton(sf::Vector2f(10, 10)); // 尺寸和位置
				sf::Texture menubuttonTexture;
				menubuttonTexture.loadFromFile("Resources/Images/menubutton.png");
				menuButton.setTexture(&menubuttonTexture); // 设置纹理
				menuButton.setPosition(140, 5); // 放在屏幕的右上角
				window.draw(menuButton);
				//Drawing the text
				scores = explode_scores + 10 * lines_cleared;

				if (lines_cleared >= 0 && lines_cleared <= 5)
				{
					draw_text(static_cast<unsigned short>(CELL_SIZE * (0.5f + COLUMNS)), static_cast<unsigned short>(0.5f * CELL_SIZE * ROWS), "Lines:" + std::to_string(lines_cleared)+"\nScores:"+std::to_string(scores) + "\nSpeed:" + std::to_string(START_FALL_SPEED / current_fall_speed) + 'x' + "\nweak", window);
				}
				else if (lines_cleared >= 6 && lines_cleared <= 10)
				{
					draw_text(static_cast<unsigned short>(CELL_SIZE * (0.5f + COLUMNS)), static_cast<unsigned short>(0.5f * CELL_SIZE * ROWS), "Lines:" + std::to_string(lines_cleared) + "\nSpeed:" + std::to_string(START_FALL_SPEED / current_fall_speed) + 'x' + "\nstill weak", window);
				}
				else
				{
					draw_text(static_cast<unsigned short>(CELL_SIZE* (0.5f + COLUMNS)), static_cast<unsigned short>(0.5f * CELL_SIZE * ROWS), "Lines:" + std::to_string(lines_cleared) + "\nSpeed:" + std::to_string(START_FALL_SPEED / current_fall_speed) + 'x' + "\ngood", window);
				}
				//Last comment in this document!
				window.display();

				//Jk, THIS is the last comment in this document
			}
		}
	}
}
