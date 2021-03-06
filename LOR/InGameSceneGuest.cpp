#include "InGameSceneGuest.h"
#include <fstream>

#define TO_DEBUG_ARCHER
//#define TO_DEBUG_ZOMBIE
//#define TO_DEBUG_KNIGHT
//#define TO_DEBUG_WICHER

//#define TO_DEBUG_MAP_SEA
#define TO_DEBUG_MAP_FOREST

InGameSceneGuest::InGameSceneGuest(HWND hwnd, Network *network) : Scene(hwnd)
{
	threadNetwork = m_network = network;
	SuspendThread(m_network->m_networkThread);

	//BGM재생
	m_network->m_nowBgmNumber = 1;
	m_network->m_gameResult = 0;
	m_network->m_gameResultBuffer = 0;

	m_network->m_system->playSound(FMOD_CHANNEL_REUSE, m_network->m_sound[1], false, &(m_network->m_channel[0]));

	sendQueueGuest.clear();
	memset(emotionNum, 0, sizeof(emotionNum));
	memset(&basicInfo, 0, sizeof(basicInfo));

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (roomInfo.m_charInfo[i] == CharacterName::Archer)
			m_characterArr[i] = Pawn(CharacterName::Archer);
		else if (roomInfo.m_charInfo[i] == CharacterName::Zombie)
			m_characterArr[i] = Pawn(CharacterName::Zombie);
		else if (roomInfo.m_charInfo[i] == CharacterName::Knight)
			m_characterArr[i] = Pawn(CharacterName::Knight);
		else if (roomInfo.m_charInfo[i] == CharacterName::Wicher)
			m_characterArr[i] = Pawn(CharacterName::Wicher);
	}

	if (roomInfo.m_mapInfo == MapName::Sea)
		m_map = new Map(0, 0, "Resource/Image/Background/Background.png");
	else if (roomInfo.m_mapInfo == MapName::Forest)
		m_map = new Map(0, 0, "Resource/Image/Background/Background_2.png");

	m_platImg[0].Load("Resource/Image/Plat/Plat_2.png");
	m_platImg[1].Load("Resource/Image/Plat/Plat_1.png");

	m_itemImg.Load("Resource/Image/UI/item_Random_2.png");

	m_winLoseImg[0].Load("Resource/Image/UI/Win.png");
	m_winLoseImg[1].Load("Resource/Image/UI/Lose.png");

	int numItem = (PLAT_MAX_NUMBER - ITEM_FIRST_IMPACT) / ITEM_INTERVAL + 1;

	m_platArr = new BaseObject[PLAT_MAX_NUMBER];
	m_itemArr = new BaseObject[numItem];

	LoadPlat();

	sendQueueGuest.push_back(NOTIFY_START);

	m_resultUICount = 0;
}

InGameSceneGuest::InGameSceneGuest()
{
}

InGameSceneGuest::~InGameSceneGuest()
{
	//ResumeThread(m_network->m_networkThread);
	if (m_map) delete m_map;
	if (m_platArr) delete[] m_platArr;
	if (m_itemArr) delete[] m_itemArr;
}


void InGameSceneGuest::Draw(HDC hdc) {
	m_map->Draw(hdc);

	float xPos = basicInfo.m_firstPlat[m_idx].xPos;
	for (int i = basicInfo.m_firstPlat[m_idx].idx, j=0; j < PLAT_SHOWN_CNT; ++i, ++j) {
		if (m_platArr[i].GetPos().y != PLAT_LOW_HEIGHT)
			m_platImg[0].TransparentBlt(hdc, xPos + PLAT_WIDTH * j, m_platArr[i].GetPos().y, PLAT_WIDTH, PLAT_HEIGHT, RGB(255, 255, 255));
		else
			m_platImg[1].TransparentBlt(hdc, xPos + PLAT_WIDTH * j, m_platArr[i].GetPos().y, PLAT_WIDTH, m_platImg[1].GetHeight(), RGB(255, 255, 255));
	}

	m_itemImg.TransparentBlt(hdc, basicInfo.m_firstItem[m_idx].xPos, m_itemArr[basicInfo.m_firstItem[m_idx].idx].GetPos().y, ITEM_SIZE, ITEM_SIZE, RGB(255, 255, 255));
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (i == m_idx) continue;
		m_characterArr[i].NetworkDrawCharacter(hdc, basicInfo.m_totalDis[m_idx], basicInfo.m_totalDis[i], basicInfo.m_yPos[i],
			basicInfo.m_imgCnt[i], basicInfo.m_state[i]);
	}
	m_characterArr[m_idx].NetworkDrawCharacter(hdc, 0, 0, basicInfo.m_yPos[m_idx],
		basicInfo.m_imgCnt[m_idx], basicInfo.m_state[m_idx]);

	m_inGameUI.DrawComboUI(hdc, basicInfo.m_combo[m_idx]);
	m_inGameUI.DrawInventoryUI(hdc, basicInfo.m_itemInfo[m_idx][0], basicInfo.m_itemInfo[m_idx][1]);

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (i == m_idx) continue;
		m_inGameUI.DrawBarUI(hdc, i, basicInfo.m_totalDis[i] / 110);
		m_inGameUI.DrawPlayerMark(hdc, i, basicInfo.m_yPos[i], basicInfo.m_totalDis[m_idx], basicInfo.m_totalDis[i]);
		if (emotionNum[i])
			m_inGameUI.DrawEmotionUI(hdc, emotionNum[i], basicInfo.m_totalDis[m_idx], basicInfo.m_totalDis[i], basicInfo.m_yPos[i]);
		else
			m_inGameUI.DrawHeadUpUI(hdc, basicInfo.m_yPos[i], basicInfo.m_totalDis[m_idx], basicInfo.m_totalDis[i], i);
	}
	m_inGameUI.DrawBarUI(hdc, m_idx, basicInfo.m_totalDis[m_idx] / 110);
	m_inGameUI.DrawPlayerMark(hdc, m_idx, basicInfo.m_yPos[m_idx]);
	if (emotionNum[m_idx])
		m_inGameUI.DrawEmotionUI(hdc, emotionNum[m_idx], 0, 0, basicInfo.m_yPos[m_idx]);
	else
		m_inGameUI.DrawHeadUpUI(hdc, basicInfo.m_yPos[m_idx], 0, 0, m_idx);

	m_inGameUI.DrawRankUI(hdc);

	if (m_network->m_gameResultBuffer == 1) m_winLoseImg[0].TransparentBlt(hdc, 200, 200, 880, 320, RGB(255, 0, 255));
	if (m_network->m_gameResultBuffer == 2) m_winLoseImg[1].TransparentBlt(hdc, 200, 200, 880, 320, RGB(0, 255, 0));

	// 이 바보야 여기있으면 어떻게
	//if (itemNum == 0)
	//	PlaySound("Resource\\Sound\\ets.wav", NULL, SND_ASYNC);
	//else if (itemNum == 1)
	//	PlaySound("Resource\\Sound\\sleeping.wav", NULL, SND_ASYNC);
	//else if (itemNum == 2)
	//	PlaySound("Resource\\Sound\\shield.wav", NULL, SND_ASYNC);
	//else if (itemNum == 3)
	//	PlaySound("Resource\\Sound\\angle.wav", NULL, SND_ASYNC);
}

void InGameSceneGuest::Timer(const double time) {
	m_map->Update(basicInfo.m_speed[m_idx], time);

	if (m_network->m_gameResultBuffer) {
		m_resultUICount++;
		if (m_resultUICount > 300) {
			m_nextScene = SceneName::RoomGuest;
			m_isDestory = true;
		}
	}
	//ShowPawnState();	//Debug
}

bool InGameSceneGuest::KeyProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//if (m_characterArr->InsertKey(wParam)) return true;
	switch (iMessage)
	{
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_SPACE:
			sendQueueGuest.emplace_back(INPUT_JUMP);

			if (m_characterArr[m_idx].GetCharType() == CharacterName::Archer || m_characterArr[m_idx].GetCharType() == CharacterName::Wicher) {
				PlaySound("Resource\\Sound\\NotmanJump.wav", NULL, SND_ASYNC);
			}
			else if (m_characterArr[m_idx].GetCharType() == CharacterName::Zombie || m_characterArr[m_idx].GetCharType() == CharacterName::Knight) {
				PlaySound("Resource\\Sound\\manJump.wav", NULL, SND_ASYNC);
			}
			break;
		case 'G':

			break;
		case '1':
			emotionNum[m_idx] = 1;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case '2':
			emotionNum[m_idx] = 2;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case '3':
			emotionNum[m_idx] = 3;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case '4':
			emotionNum[m_idx] = 4;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case '5':
			emotionNum[m_idx] = 5;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case '6':
			emotionNum[m_idx] = 6;
			sendQueueGuest.emplace_back(INPUT_EMOTION);
			break;
		case 'Q':
		case 'q':
			if(basicInfo.m_itemInfo[m_idx][0] != -1)
				sendQueueGuest.emplace_back(INPUT_KEY_Q);
			break;
		case 'W':
		case 'w':
			if (basicInfo.m_itemInfo[m_idx][1] != -1)
				sendQueueGuest.emplace_back(INPUT_KEY_W);
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

bool InGameSceneGuest::MouseProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	return true;
}

void InGameSceneGuest::Destory() {

}



void InGameSceneGuest::LoadPlat() {
	ifstream inFile("Resource/Data/NewPlatData.txt", ios::in);
	//to debug
	//ifstream inFile("platData.txt", ios::in);	

	int posX = 1;
	int posY = 0;

	for (int i = 0, j = 0; i < PLAT_MAX_NUMBER; i++) {

		inFile >> posY;

		m_platArr[i].SetPos(posX * i *  PLAT_WIDTH, (posY * (-100)) + PLAT_LOW_HEIGHT);
		if (i % ITEM_INTERVAL == ITEM_FIRST_IMPACT)
		{
			m_itemArr[j].SetPos(3000, (posY * (-100)) + PLAT_LOW_HEIGHT - PLAT_ITEM_HEIGHT);
			j++;
		}
		#ifdef DEBUG_MODE
				//to debug
				std::cout << i << "번째  " << posY << "   " << m_platArr[i].GetPos().x << "   " << m_platArr[i].GetPos().y << std::endl;
		#endif
	}
	inFile.close();

	basicInfo.m_firstItem[m_idx].xPos = 3000;
	basicInfo.m_itemInfo[m_idx][0] = basicInfo.m_itemInfo[m_idx][1] = -1;
}

void InGameSceneGuest::ShowPawnState() {

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