#pragma once

#include "Scene.h"

class RoomSceneGuest : public Scene
{
public:
	RoomSceneGuest();
	RoomSceneGuest(HWND, Network*);
	~RoomSceneGuest();

public:
	virtual void Draw(HDC hdc);
	virtual void Timer(const double count);

	virtual bool MouseProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);


public:
	void LoadCImage();
	void DrawCharacter(HDC hdc);
	int GetXPosByIdx(int idx);
	int GetYPosByIdx(int idx);

private:
	CImage			m_backImg[2];
	CImage			m_ImgArcher;
	CImage			m_ImgZombie;
	CImage			m_ImgKnight;
	CImage			m_ImgWicher;

public:
	virtual MapName		GetMapName() { return roomInfo.m_mapInfo; }
	virtual CharacterName* GetCharacterName() { return roomInfo.m_charInfo; }
};