#include "InGameScene.h"
#include <fstream>

#define TO_DEBUG_ARCHER
//#define TO_DEBUG_ZOMBIE
//#define TO_DEBUG_KNIGHT
//#define TO_DEBUG_WICHER

//#define TO_DEBUG_MAP_SEA
#define TO_DEBUG_MAP_FOREST


InGameScene::InGameScene(HWND hwnd) : Scene(hwnd)
{
#ifdef TO_DEBUG_ARCHER
	m_characterArr[0] = Pawn(CharacterName::Archer);
#endif

#ifdef TO_DEBUG_ZOMBIE
	m_characterArr[0] = Pawn(CharacterName::Zombie);
#endif

#ifdef TO_DEBUG_KNIGHT
	m_characterArr[0] = Pawn(CharacterName::Knight);
#endif

#ifdef TO_DEBUG_WICHER
	m_characterArr[0] = Pawn(CharacterName::Wicher);
#endif

	m_characterArr[0].SetState(State::Fall);

#ifdef TO_DEBUG_MAP_SEA
	m_map = new Map(0, 0, "Resource/Image/Background/Background.png");
#endif 

#ifdef TO_DEBUG_MAP_FOREST
	m_map = new Map(0, 0, "Resource/Image/Background/Background_2.png");
#endif 
	m_platImg[0] = new CImage;
	m_platImg[1] = new CImage;

	m_platImg[0]->Load("Resource/Image/Plat/Plat_2.png");
	m_platImg[1]->Load("Resource/Image/Plat/Plat_1.png");

	m_numPlat = PLAT_MAX_NUMBER;
	m_platArr = new BaseObject[m_numPlat];
	LoadPlat();

	m_inGameUI = new InGameSceneUI;
}

InGameScene::InGameScene(HWND hwnd, MapName insertMap, CharacterName* insertCharacter) : Scene(hwnd)
{
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (insertCharacter[i] == CharacterName::Archer)
			m_characterArr[i] = Pawn(CharacterName::Archer);
		else if (insertCharacter[i] == CharacterName::Zombie)
			m_characterArr[i] = Pawn(CharacterName::Zombie);
		else if (insertCharacter[i] == CharacterName::Knight)
			m_characterArr[i] = Pawn(CharacterName::Knight);
		else if (insertCharacter[i] == CharacterName::Wicher)
			m_characterArr[i] = Pawn(CharacterName::Wicher);
	}
	
	
	m_characterArr->SetState(State::Fall);
	
	if (insertMap == MapName::Sea)
		m_map = new Map(0, 0, "Resource/Image/Background/Background.png");
	else if (insertMap == MapName::Forest)
		m_map = new Map(0, 0, "Resource/Image/Background/Background_2.png");
	
	m_platImg[0] = new CImage;
	m_platImg[1] = new CImage;

	m_platImg[0]->Load("Resource/Image/Plat/Plat_2.png");
	m_platImg[1]->Load("Resource/Image/Plat/Plat_1.png");

	m_numPlat = PLAT_MAX_NUMBER;
	m_platArr = new BaseObject[m_numPlat];
	LoadPlat();

	m_inGameUI = new InGameSceneUI;
}

InGameScene::InGameScene() 
{
}

InGameScene::~InGameScene()
{
}



void InGameScene::Draw(HDC hdc) {
	m_map->Draw(hdc);

	for (int i = 0; i < m_numPlat; i++) {
		if(m_platArr[i].GetPos().y != PLAT_LOW_HEIGHT)
			m_platImg[0]->TransparentBlt(hdc, m_platArr[i].GetPos().x, m_platArr[i].GetPos().y, PLAT_WIDTH, PLAT_HEIGHT, RGB(255, 255, 255));
		else
			m_platImg[1]->TransparentBlt(hdc, m_platArr[i].GetPos().x, m_platArr[i].GetPos().y, PLAT_WIDTH, m_platImg[1]->GetHeight() , RGB(255,255,255));
	}

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		m_characterArr[i].Draw(hdc, m_characterArr[i].GetTotalDistance() - m_characterArr[0].GetTotalDistance(), m_characterArr[i].GetState());
	}
	m_inGameUI->DrawComboUI(hdc, m_characterArr[0].GetCombo());
	m_inGameUI->DrawBarUI(hdc, m_characterArr[0].GetTotalDistance()/100);
	m_inGameUI->DrawInventoryUI(hdc, 0, 0);

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (m_emotionNumber)
			m_inGameUI->DrawEmotionUI(hdc, m_emotionNumber, m_characterArr[i].GetPos().x, m_characterArr[i].GetPos().y);
		else
			m_inGameUI->DrawHeadUpUI(hdc, m_characterArr[i].GetPos().y);
	}
	

}

void InGameScene::Timer(const double time){

	m_map->Update(m_characterArr[0].GetSpeed(), time);

	for(int i=0; i<MAX_PLAYER; ++i)
		m_characterArr[i].Update(m_characterArr[i].GetState(), time);


	for( int i = 0 ; i < PLAT_MAX_NUMBER; i++)
		m_platArr[i].Update(m_characterArr[0].GetSpeed(), time);

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		ComputePawn(i);
	}
	
	EmotionUIProc();
	//ShowPawnState();	//Debug
}

bool InGameScene::KeyProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//if (m_characterArr->InsertKey(wParam)) return true;
	switch (iMessage)
	{
	case WM_KEYDOWN:
	{
		if (m_characterArr[0].InsertKey(wParam))return true;
		
		switch (wParam) {
		case VK_SPACE:
		
			break;
		case 'G':

			break;
		case '1':
			m_emotionNumber = 1;
			m_emotionTimer = 1;
			break;
		case '2':
			m_emotionNumber = 2;
			m_emotionTimer = 1;
			break;
		case '3':
			m_emotionNumber = 3;
			m_emotionTimer = 1;
			break;
		case '4':
			m_emotionNumber = 4;
			m_emotionTimer = 1;
			break;
		case '5':
			m_emotionNumber = 5;
			m_emotionTimer = 1;
			break;
		case '6':
			m_emotionNumber = 6;
			m_emotionTimer = 1;
			break;
		}
	}
	case WM_KEYUP:
	{
		switch (wParam)
		{
			break;
		}
		return false;
	}
	}
}

bool InGameScene::MouseProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	return true;
}

void InGameScene::Destory() {

}



void InGameScene::LoadPlat() {
	//ifstream inFile("Resource/Data/platData.txt", ios::in);
	ifstream inFile("Resource/Data/NewPlatData.txt", ios::in);
	//to debug
	//ifstream inFile("platData.txt", ios::in);	
	
	int posX = 1;
	int posY = 0;
	
	for (int i = 0; i < m_numPlat; i++) {
		
		inFile >> posY;
		
		m_platArr[i].SetPos(posX * i *  PLAT_WIDTH, (posY * (-100)) + PLAT_LOW_HEIGHT);
	
		//to debug
		//std::cout << i << "번째  " << posY << "   " << m_platArr[i].GetPos().x << "   " << m_platArr[i].GetPos().y << std::endl;
	}
	inFile.close();
	/*
	FILE *fp;

	fp = fopen("Resource/Data/platData.txt", "r");

	int posX = 1;
	int posY = 0;

	for(int i = 0; i < m_numPlat; i++){ 
		fscanf(fp, "%d", &posY);
		m_platArr[i].SetPos(posX * i * 100, (posY * (-100)) + 600);
		std::cout << posY << std::endl;
	}

	std::fclose(fp);
	*/
}

void InGameScene::ComputePawn(int idx) {
	//std::cout << m_characterArr->GetTotalDistance() / 100 + 2 << std::endl;
#pragma region [캐릭터의 Y값을 계산합니다.]

	int leftPlat = (int)(m_characterArr[idx].GetTotalDistance() / PLAT_WIDTH + 2);
	int rightPlat = (int)(m_characterArr[idx].GetTotalDistance() / PLAT_WIDTH + 3);

	float pawnPosY = m_characterArr->GetPos().y + 185;

	if (m_characterArr[idx].GetState() == State::Fall
		|| m_characterArr[idx].GetState() == State::JumpEnd
		|| m_characterArr[idx].GetState() == State::DoubleJumpEnd
		) {
		//if (m_platArr[leftPlat].GetPos().y - 5<= pawnPosY
		//	&& m_platArr[leftPlat].GetPos().y + 5 > pawnPosY) {
		//
		//	m_characterArr->SetPos(200, m_platArr[leftPlat].GetPos().y - 185);
		//	m_characterArr->SetState(State::Run);
		//	m_characterArr->GetUnit().SetImageCount(0);
		//}
		//else 
			
		if (m_platArr[rightPlat].GetPos().y - 10 <= pawnPosY
			&& m_platArr[rightPlat].GetPos().y + 30 > pawnPosY) {

			m_characterArr[idx].SetPos(200, m_platArr[rightPlat].GetPos().y - 185);
			m_characterArr[idx].SetState(State::Run);
			m_characterArr[idx].GetUnit().SetImageCount(0);
			m_characterArr[idx].ResetFallSpeed();
		}
		else if (pawnPosY > SCREEN_HEIGHT + 50 ){
			m_characterArr[idx].ResetCombo();
			m_characterArr[idx].SetPos(200, 0);
			m_characterArr[idx].SetState(State::JumpEnd);
		}
	}
	else if (m_characterArr[idx].GetState() == State::Run) {
		
		bool isValue = false;
		//if (m_platArr[leftPlat].GetPos().y - 5 <= pawnPosY
		//	&& m_platArr[leftPlat].GetPos().y + 5 > pawnPosY) {
		//}
		//else 
			if (m_platArr[rightPlat].GetPos().y - 10 <= pawnPosY
			&& m_platArr[rightPlat].GetPos().y + 30 > pawnPosY) {
		}
		else {
			m_characterArr[idx].SetState(State::Fall);
		}
	}
		//else m_characterArr->SetState(State::Fall);
		//to debug
#pragma endregion

}

void InGameScene::ShowPawnState()  {

	if (m_characterArr[0].GetState() == State::Run) {
		std::cout << "Run" << std::endl;
	}
	else if (m_characterArr[0].GetState() == State::JumpLoop) {
		std::cout << "JUMPLOOP" << std::endl;
	}
	else if (m_characterArr[0].GetState() == State::JumpEnd) {
		std::cout << "JUMPEnd" << std::endl;
	}
	else if (m_characterArr[0].GetState() == State::DoubleJumpEnd) {
		std::cout << "DoubleJUMPEnd" << std::endl;
	}

	else if (m_characterArr[0].GetState() == State::Fall) {
		std::cout << "Fall" << std::endl;
	}
	else {
		if (m_characterArr[0].GetState() == State::Boost) {
			std::cout << "Boost" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::Collide) {
			std::cout << "Collide" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::Death) {
			std::cout << "Death" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::DoubleJumpLoop) {
			std::cout << "DoupleJump" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::Sleep) {
			std::cout << "Sleep" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::Slow) {
			std::cout << "Slow" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::Stun) {
			std::cout << "Stun" << std::endl;
		}
		else if (m_characterArr[0].GetState() == State::TripleJump) {
			std::cout << "TripleJump" << std::endl;
		}
		else
			std::cout << "What is the Problem!!" << std::endl;
	}
}

void InGameScene::EmotionUIProc() {
	if (m_emotionNumber) {
		m_emotionTimer++;

		if (m_emotionTimer >= 90) {
			m_emotionTimer = 0;
			m_emotionNumber = 0;
		}
	}
}
