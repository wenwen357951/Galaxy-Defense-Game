#include <SFML/Graphics.hpp>
#include <ctime>
#include <list>
#include <iostream>
#include <fstream>
#include <string>

const int Width = 1200, Height = 800;
const float DegToRad = 0.017453f;
sf::Font font;

int game_time = 60;
int bullet_per_sec = 5;
int bullet_give_on_start = 80;
int start_countDown_time = 6;
int score_rock = 1;
int score_ship = -5;
int top_score = 0;
int move_cd = 1;
int summon_rock_num = 15;

class Animation {
public:
	Animation() {
	};

	Animation(sf::Texture& t, int x, int y, int width, int height, int count, float speed) {
		frame = 0;
		this->speed = speed;
		for (int i = 0; i < count; i++) {
			frames.push_back(sf::IntRect(x + i * width, y, width, height));
		}
		sprite.setTexture(t);
		sprite.setOrigin(width / 2, height / 2);
		sprite.setTextureRect(frames[0]);
	};

	void update() {
		frame += speed;
		int n = frames.size();
		if (frame >= n) {
			frame -= n;
		}
		if (n > 0) {
			sprite.setTextureRect(frames[int(frame)]);
		}
	};

	bool isEnd() {
		return frame + speed >= frames.size();
	}

public:
	float frame, speed;
	sf::Sprite sprite;
	std::vector<sf::IntRect> frames;
};

class Entity {
public:
	Entity() {
		life = true;
	}

	void setting(Animation& anima, int x, int y, float ang = 0, int rad = 1) {
		this->x = x;
		this->y = y;
		this->ang = ang;
		this->rad = rad;
		this->anima = anima;
	}

	void draw(sf::RenderWindow& app) {
		anima.sprite.setPosition(x, y);
		anima.sprite.setRotation(ang + 90);
		app.draw(anima.sprite);
	}

public:
	virtual void update() {
	};

	virtual ~Entity() {
	};

public:
	float x, y;
	float dx, dy;
	float rad, ang;
	bool life;
	std::string name;
	Animation anima;
};

class Bullet : public Entity {
public:
	Bullet() {
		name = "bullet";
	}

	void  update() {
		dx = cos(ang * DegToRad) * 8;
		dy = sin(ang * DegToRad) * 8;
		x += dx;
		y += dy;

		if (x > Width || x < 0 || y > Height || y < 0) {
			life = false;
		}
	}

};

class asteroid : public Entity {
public:
	asteroid() {
		dx = rand() % 8 - 4;
		dy = rand() % 8 - 4;
		name = "asteroid";
	}

	void  update() {
		x += dx;
		y += dy;

		if (x > Width) {
			x = 0;
		}

		if (x < 0) {
			x = Width;
		}

		if (y > Height) {
			y = 0;
		}

		if (y < 0) {
			y = Height;
		}
	}

};

class Smoke : public Entity {
public:
	Smoke() {
		name = "smoke";
	}

	void  update() {
		if (anima.isEnd()) {
			life = false;
		}
	}
};

class Player : public Entity {
public:
	bool thrust;

	Player() {
		name = "player";
	}

	void update() {
		if (thrust) {
			dx += cos(ang * DegToRad) * 0.2;
			dy += sin(ang * DegToRad) * 0.2;
		}
		else {
			dx *= 0.99;
			dy *= 0.99;
		}

		int maxSpeed = 15;
		float speed = sqrt(dx * dx + dy * dy);
		if (speed > maxSpeed) {
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}

		x += dx;
		y += dy;

		if (x > Width) {
			x = 0;
		}
		if (x < 0) {
			x = Width;
		}
		if (y > Height) {
			y = 0;
		}
		if (y < 0) {
			y = Height;
		}
	}
};

bool isCollide(Entity *a, Entity *b) {
	return (b->x - a->x)*(b->x - a->x) +
		(b->y - a->y)*(b->y - a->y) <
		(a->rad + b->rad)*(a->rad + b->rad);
}

class Menu {
public:
	int count;
	int selectedItemIndex;
	sf::Text menu[5];

public:
	void draw(sf::RenderWindow &window) {
		for (int i = 0; i < count; i++) {
			window.draw(menu[i]);
		}
	}

	void MoveUp() {
		if (selectedItemIndex - 1 >= 0) {
			menu[selectedItemIndex].setFillColor(sf::Color::White);
			selectedItemIndex--;
			menu[selectedItemIndex].setFillColor(sf::Color::Red);
		}
	}
	void MoveDown() {
		if (selectedItemIndex + 1 < count) {
			menu[selectedItemIndex].setFillColor(sf::Color::White);
			selectedItemIndex++;
			menu[selectedItemIndex].setFillColor(sf::Color::Red);
		}
	}

	int GetPressedItem() {
		return selectedItemIndex;
	}
};

class MainMenu : public Menu {
public:
	MainMenu(float width, float height, float x, float y) {
		count = 3;
		menu[0].setFont(font);
		menu[0].setFillColor(sf::Color::Red);
		menu[0].setString("Play");
		menu[0].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 1));

		menu[1].setFont(font);
		menu[1].setFillColor(sf::Color::White);
		menu[1].setString("Options");
		menu[1].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 2));

		menu[2].setFont(font);
		menu[2].setFillColor(sf::Color::White);
		menu[2].setString("Exit");
		menu[2].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 3));
		selectedItemIndex = 0;
	};
};

class OptionMenu : public Menu {
public:
	OptionMenu(float width, float height, float x, float y) {
		count = 4;
		menu[0].setFont(font);
		menu[0].setFillColor(sf::Color::Red);
		menu[0].setString("Starting bullets: " + std::to_string(bullet_give_on_start));
		menu[0].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 1));

		menu[1].setFont(font);
		menu[1].setFillColor(sf::Color::White);
		menu[1].setString("Game Time: " + std::to_string(game_time));
		menu[1].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 2));

		menu[2].setFont(font);
		menu[2].setFillColor(sf::Color::White);
		menu[2].setString("Asteroid Score: " + std::to_string(score_rock));
		menu[2].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 3));

		menu[3].setFont(font);
		menu[3].setFillColor(sf::Color::White);
		menu[3].setString("Back");
		menu[3].setPosition(sf::Vector2f(x + width / 3, y + height / (count + 1) * 4));
		selectedItemIndex = 0;
	};
};

void reset(std::list<Entity*>& entities, Player* player, Animation& play_anim, Animation& rock_anim) {
	entities.clear();
	player->setting(play_anim, 200, 200, 0, 20);
	entities.push_back(player);
	for (int i = 0; i < summon_rock_num; i++) {
		asteroid *a = new asteroid();
		a->setting(rock_anim, rand() % Width, rand() % Height, rand() % 360, 25);
		while (isCollide(a, player)) {
			a->setting(rock_anim, rand() % Width, rand() % Height, rand() % 360, 25);
		}
		entities.push_back(a);
	}
}

void loadConfig() {
	std::ifstream config("config.txt");
	if (config.is_open()) {
		std::streambuf *cinbuf = std::cin.rdbuf();
		std::cin.rdbuf(config.rdbuf());
		std::cin
			>> game_time
			>> bullet_per_sec
			>> bullet_give_on_start
			>> start_countDown_time
			>> score_rock
			>> score_ship
			>> top_score
			>> move_cd
			>> summon_rock_num;
		std::cin.rdbuf(cinbuf);
		config.close();
	}
}

void saveConfig() {
	std::ofstream config("config.txt");
	std::streambuf *coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(config.rdbuf());

	std::cout << game_time << std::endl;
	std::cout << bullet_per_sec << std::endl;
	std::cout << bullet_give_on_start << std::endl;
	std::cout << start_countDown_time << std::endl;
	std::cout << score_rock << std::endl;
	std::cout << score_ship << std::endl;
	std::cout << top_score << std::endl;
	std::cout << move_cd << std::endl;
	std::cout << move_cd << std::endl;

	std::cout.rdbuf(coutbuf);
}

int main() {
	srand(time(0));
	loadConfig();
	saveConfig();
	// �e���]�m
	sf::ContextSettings set;
	set.antialiasingLevel = 8;
	// �Ыص���
	sf::RenderWindow app(sf::VideoMode(Width, Height), "Game", sf::Style::Default, set);
	// ��������V��
	app.setFramerateLimit(60);
	// ����}�C
	std::list<Entity*> entities;
	// �Ыا���
	sf::Texture spaceship_T;
	sf::Texture fire_T;
	sf::Texture smoke_T;
	sf::Texture rock1_T;
	sf::Texture rock2_T;
	sf::Texture explosions1_T;
	sf::Texture explosions2_T;
	sf::Texture Background_T;
	// Ū������
	spaceship_T.loadFromFile("images/spaceship.png");
	fire_T.loadFromFile("images/fire_red.png");
	smoke_T.loadFromFile("images/smoke.png");
	rock1_T.loadFromFile("images/rock1.png");
	rock2_T.loadFromFile("images/rock2.png");
	explosions1_T.loadFromFile("images/explosions1.png");
	explosions2_T.loadFromFile("images/explosions2.png");
	Background_T.loadFromFile("images/bg.jpg");

	sf::Sprite background_S(Background_T);

	Animation player_A(spaceship_T, 40, 0, 40, 40, 1, 0);
	Animation player_go_A(spaceship_T, 40, 40, 40, 40, 1, 0);
	Animation bullet_A(fire_T, 0, 0, 32, 64, 16, 0.8);
	Animation smoke_A(smoke_T, 0, 0, 32, 32, 17, 0.9);
	Animation rock1_A(rock1_T, 0, 0, 64, 64, 16, 0.2);
	Animation rock2_A(rock2_T, 0, 0, 64, 64, 16, 0.2);
	Animation explosion(explosions1_T, 0, 0, 256, 256, 48, 0.5);
	Animation ship_explo(explosions2_T, 0, 0, 192, 192, 64, 0.5);

	font.loadFromFile("font/Marker Notes.ttf");

	// �Ыت��a
	Player *player = new Player();

	reset(entities, player, player_A, rock1_A);

	sf::Clock clock;
	float timer = 0, delay = 1;
	bool gameStart = false;
	bool start_countDown = false;
	int scd; // start countDown

	sf::RectangleShape background(sf::Vector2f(Width, Height));
	sf::RectangleShape box(sf::Vector2f(400, 400));
	box.setPosition(Width / 2 - 200, Height / 2 - 200);
	box.setFillColor(sf::Color::Color(0, 0, 0, 200));
	box.setOutlineColor(sf::Color::Color(128, 128, 128, 255));
	box.setOutlineThickness(0.8f);
	background.setPosition(0, 0);
	background.setFillColor(sf::Color::Color(0, 0, 0, 150));
	MainMenu* menu = new MainMenu(box.getSize().x, box.getSize().y, box.getPosition().x - 20, box.getPosition().y);
	OptionMenu* option = new OptionMenu(box.getSize().x, box.getSize().y, box.getPosition().x - 100, box.getPosition().y);

	sf::Text countDown;
	sf::Text bullet;
	sf::Text score;

	countDown.setFont(font);
	bullet.setFont(font);
	score.setFont(font);

	int score_num = 0;
	int bullet_num = 0;
	int game_count = 0;
	bool score_top = false;

	bool canMove = false;
	int move = 1;

	Menu* now_menu = menu;

	// �P�_�����O�_���}
	while (app.isOpen()) {
		float time = clock.getElapsedTime().asSeconds();
		clock.restart();
		timer += time;
		sf::Event event;
		// �ƥ�Ĳ�o
		while (app.pollEvent(event)) {
			// �����ƥ�
			if (event.type == sf::Event::Closed) {
				app.close();
			}
			if (gameStart) {
				if (sf::Event::KeyPressed == event.type) {
					if (sf::Keyboard::Space == event.key.code && canMove) {
						if (bullet_num > 0) {
							bullet_num -= 1;
							Bullet *b = new Bullet();
							b->setting(bullet_A, player->x, player->y, player->ang, 10);
							entities.push_back(b);
						}
					}
				}
			}
			if (!gameStart && !start_countDown) {
				if (sf::Event::KeyPressed == event.type) {
					// �W��
					if (event.key.code == sf::Keyboard::Up) {
						now_menu->MoveUp();
					}
					// �U��
					if (event.key.code == sf::Keyboard::Down) {
						now_menu->MoveDown();
					}
					if (now_menu->count == 4) {
						switch (now_menu->GetPressedItem()) {
						case 0:
							// ����
							if (event.key.code == sf::Keyboard::Left) {
								bullet_give_on_start -= 1;
							}
							// �k��
							if (event.key.code == sf::Keyboard::Right) {
								bullet_give_on_start += 1;
							}
							now_menu->menu[0].setString("Starting bullets: " + std::to_string(bullet_give_on_start));
							break;
						case 1:
							// ����
							if (event.key.code == sf::Keyboard::Left) {
								game_time -= 1;
							}
							// �k��
							if (event.key.code == sf::Keyboard::Right) {
								game_time += 1;
							}
							now_menu->menu[1].setString("Game Time: " + std::to_string(game_time));
							break;
						case 2:
							// ����
							if (event.key.code == sf::Keyboard::Left) {
								score_rock -= 1;
							}
							// �k��
							if (event.key.code == sf::Keyboard::Right) {
								score_rock += 1;
							}
							now_menu->menu[2].setString("Asteroid Score: " + std::to_string(score_rock));
							break;
						}
					}
					// ��J��
					if (event.key.code == sf::Keyboard::Enter) {
						// �P�_��ܪ�����
						switch (now_menu->GetPressedItem()) {
						case 0:
							if (now_menu->menu[0].getString() == "Play") {
								scd = start_countDown_time;
								score_num = 0;
								bullet_num = bullet_give_on_start;
								game_count = game_time;
								start_countDown = true;
								score_top = false;
								countDown.setString("");
								canMove = true;
								move = move_cd;
							}
							break;
						case 1:
							if (now_menu->menu[1].getString() == "Options") {
								now_menu = option;
							}
							break;
						case 2:
							if (now_menu->menu[2].getString() == "Exit") {
								app.close();
							}
							break;
						case 3:
							saveConfig();
							menu->MoveUp();
							now_menu = menu;
							option->MoveUp();
							option->MoveUp();
							option->MoveUp();
						}
					}
				}
			}
		}

		if (timer > delay) {
			if (start_countDown) {
				scd -= 1;
				if (scd == -1) {
					start_countDown = false;
				}
				else if (scd == 0) {
					countDown.setString("Go!");
					gameStart = true;
				}
				else {
					countDown.setString(std::to_string(scd));
				}
			}
			if (gameStart) {
				if (!canMove) {
					if (move <= 0) canMove = true;
					move -= 1;
				}
				if (game_count <= 0) {
					gameStart = false;
					reset(entities, player, player_A, rock1_A);
					if (score_num > top_score) {
						top_score = score_num;
						score_top = true;
					}
					saveConfig();
				}
				else {
					bullet_num += bullet_per_sec;
					game_count -= 1;
				}
			}
			timer = 0;
		}

		if (gameStart) {
			// ����ƥ�
			// �k
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && canMove) {
				player->ang += 3;
			}
			// ��
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && canMove) {
				player->ang -= 3;
			}
			// �W
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && canMove) {
				player->thrust = true;
				Smoke *smoke = new Smoke();
				smoke->setting(smoke_A, player->x, player->y, rand() % 360);
				entities.push_back(smoke);
			}
			else {
				player->thrust = false;
			}
			// �I���P�_
			for (auto a : entities) {
				if (a->life == false) {
					continue;
				}
				for (auto b : entities) {
					if (b->life == false) {
						continue;
					}
					// �l�u�P�p���P�I��
					if (a->name == "asteroid" && b->name == "bullet") {
						if (isCollide(a, b)) { // �P�_�O�_�I��
							// �]�m�s�����_
							a->life = false;
							b->life = false;
							// �b���e�y�Щ�m�z���S��
							Entity *explo = new Entity();
							explo->setting(explosion, a->x, a->y);
							explo->name = "explosion";
							entities.push_back(explo);
							// �ͦ��p�k��
							for (int i = 0; i < 2; i++) {
								if (a->rad == 15) continue;
								Entity *e = new asteroid();
								e->setting(rock2_A, a->x, a->y, rand() % 360, 15);
								entities.push_back(e);
							}
							score_num += score_rock;
						}
					}
					// ���a�P�p���P�I��
					if (a->name == "player" && b->name == "asteroid" && canMove) {
						if (isCollide(a, b)) { // �P�_�O�_�I��
							// �]�m�s�����_
							b->life = false;
							// �b���e�y�Щ�m�z���S��
							Entity *e = new Entity();
							e->setting(ship_explo, a->x, a->y);
							e->name = "explosion";
							entities.push_back(e);
							// �ͦ����a
							player->setting(player_A, Width / 2, Height / 2, 0, 20);
							player->dx = 0; player->dy = 0;
							score_num += score_ship;
							canMove = false;
							move = move_cd;
						}
					}
				}
			}
			// �������O�K��
			if (player->thrust) {
				player->anima = player_go_A;
			}
			else {
				player->anima = player_A;
			}
			// ���z���S�ļ��񧹵����ʵe�ةR
			for (auto e : entities) {
				if (e->name == "explosion" || e->name == "smoke") {
					if (e->anima.isEnd()) e->life = false;
				}
			}
			if (rand() % 150 == 0) {
				asteroid *a = new asteroid();
				a->setting(rock1_A, 0, rand() % Height, rand() % 360, 25);
				entities.push_back(a);
			}
			// ��s����
			for (auto i = entities.begin(); i != entities.end();) {
				Entity *e = *i;
				e->update();
				e->anima.update();
				if (e->life == false) {
					i = entities.erase(i);
					delete e;
				}
				else {
					i++;
				}
			}
		}
		// �M���e��
		app.clear(sf::Color::Black);
		background_S.setPosition(0, 0);
		app.draw(background_S);
		// ø�s����
		for (auto enity : entities) {
			if (enity->name == "player") {
				if (canMove ||
					move <= 1) {
					enity->draw(app);
				}
			}
			else {
				enity->draw(app);
			}
		}
		if (gameStart) {
			score.setPosition(Width / 4 * 3, 0);
			score.setString("Score: " + std::to_string(score_num));
			countDown.setPosition(Width / 4 * 2, 0);
			countDown.setString("Time: " + std::to_string(game_count));
			bullet.setPosition(Width / 4 * 1, 0);
			bullet.setString("Ammo: " + std::to_string(bullet_num));
			score.setCharacterSize(24);
			score.setFillColor(sf::Color::Yellow);
			bullet.setCharacterSize(24);
			bullet.setFillColor(sf::Color::Green);
			countDown.setCharacterSize(24);
			countDown.setFillColor(sf::Color::White);
			app.draw(score);
			app.draw(bullet);
			app.draw(countDown);
		}
		else if (start_countDown) {
			countDown.setFillColor(sf::Color::Yellow);
			countDown.setCharacterSize(50);
			countDown.setPosition(Width / 2, Height / 2);
			app.draw(countDown);
		}
		else {
			app.draw(background);
			app.draw(box);
			now_menu->draw(app);
			if (score_num != 0) {
				score.setPosition(Width / 4 * 2, 0);
				score.setString("Last Score: " + std::to_string(score_num));
				score.setCharacterSize(24);
				score.setFillColor(sf::Color::White);
				app.draw(score);
			}
			if (score_top) {
				score.setPosition(Width / 4 * 1 - 20, 100);
				score.setString("Congratulate! You get the highest score.");
				score.setCharacterSize(24);
				score.setFillColor(sf::Color::Yellow);
				app.draw(score);
			}
			score.setPosition(Width / 4 * 1, 0);
			score.setString("Top Score: " + std::to_string(top_score));
			score.setCharacterSize(24);
			score.setFillColor(sf::Color::White);
			app.draw(score);
		}
		// ��ܵ���
		app.display();
	}
	return 0;
}