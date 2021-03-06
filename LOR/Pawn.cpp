#include "Pawn.h"

Pawn::Pawn()
{
}

Pawn::Pawn(CharacterName inputCharacterName)
{
	if (inputCharacterName == CharacterName::Archer)
	{
		m_unit = new CArcher;
		m_charType = CharacterName::Archer;
	}
	else if (inputCharacterName == CharacterName::Zombie)
	{
		m_unit = new CZombie;
		m_charType = CharacterName::Zombie;
	}
	else if (inputCharacterName == CharacterName::Knight)
	{
		m_unit = new CKnight;
		m_charType = CharacterName::Knight;
	}
	else if (inputCharacterName == CharacterName::Wicher)
	{
		m_unit = new CWicher;
		m_charType = CharacterName::Wicher;
	}

	m_baseSpeed = m_unit->GetBaseSpeed();
	m_state = State::Fall;
	
	ResetBaseSpeed();
	ResetFallSpeed();
	ResetJumpSpeed();
	
	m_pos.x = INIT_PAWN_POS_X;
}

Pawn::Pawn(float x, float y) : BaseObject(x, y)
{

}

Pawn::~Pawn()
{
}

void Pawn::Update(State state, double time) {

	ComputeTotalDistance();
	ProcessCombo();
	ProcessGravity();
	ProcessJump();

	m_unit->Update(state, time);
}
//void Pawn::Draw(HDC hdc, float x, float y, float sizeX, float sizeY) {
//	m_unit->Draw(hdc, m_pos.x, m_pos.y, m_size.x, m_size.y, );
//}

void Pawn::Draw(HDC hdc, float xDiff, State state)
{
	m_unit->Draw(hdc, m_pos.x + xDiff, m_pos.y, m_size.x, m_size.y, state);
}

void Pawn::Draw(HDC hdc, State state) {
	m_unit->Draw(hdc, m_pos.x, m_pos.y, m_size.x, m_size.y, state);
}

void Pawn::ComputeTotalDistance() {
	m_totalDistance += m_speed * m_bufSpeed;
}

void Pawn::ProcessCombo() {

	m_combo = (m_totalDistance - m_comboStandard) / 100;

	if (m_combo % COMBO_COUNT_MAX == 1)
		m_comboEffect = true;

	if (m_comboEffect) {
		if (m_combo != 0 && m_combo % COMBO_COUNT_MAX == 0 && m_bufSpeed != 0) {
			m_speed = m_speed + m_baseSpeed / SPEEDUP_DENOMINATOR;
			m_comboEffect = false;
		}
	}
}

bool Pawn::InsertKey(WPARAM Key) {
	if (Key == VK_SPACE) {
		if (m_state == State::Run) {


			if (m_charType == CharacterName::Archer || m_charType == CharacterName::Wicher) {
				PlaySound("Resource\\Sound\\NotmanJump.wav", NULL, SND_ASYNC);
			}
			else if (m_charType == CharacterName::Zombie || m_charType == CharacterName::Knight) {
				PlaySound("Resource\\Sound\\manJump.wav", NULL, SND_ASYNC);
			}

			m_state = State::JumpStart;
			ResetJumpSpeed();
			m_unit->SetImageCount(m_unit->GetJumpImageCount());
			m_state = State::JumpLoop;

			return true;
		}	
		else if (m_state == State::JumpLoop || m_state == State::JumpEnd) {

			if (m_charType == CharacterName::Archer && m_charType == CharacterName::Wicher) {
				PlaySound("Resource\\Sound\\NotmanJump.wav", NULL, SND_ASYNC);
			}
			else if (m_charType == CharacterName::Zombie && m_charType == CharacterName::Knight) {
				PlaySound("Resource\\Sound\\manJump.wav", NULL, SND_ASYNC);
			}

			ResetJumpSpeed();
			m_state = State::DoubleJumpStart;
			ResetFallSpeed();
			m_unit->SetImageCount(m_unit->GetJumpImageCount());
			m_state = State::DoubleJumpLoop;
			return true;
		}		
		else if (m_state == State::DoubleJumpLoop || m_state == State::DoubleJumpEnd) {
			if (m_charType == CharacterName::Archer) {
				ResetJumpSpeed();
				m_state = State::TripleJumpStart;
				ResetFallSpeed();
				m_unit->SetImageCount(m_unit->GetJumpImageCount());
				m_state = State::TripleJumpLoop;
				return true;
			}
		}
	}
	return false;
}

void Pawn::ProcessGravity() {
	if (m_state == State::Fall || m_state == State::JumpEnd || m_state == State::DoubleJumpEnd || m_state == State::TripleJumpEnd) {
		m_pos.y += m_fallSpeed;
		//m_fallSpeed *= 1.05;
		m_fallSpeed *= 1.1;

		if (m_fallSpeed >= 15.0f)
			m_fallSpeed = 14.9f;
	}
}

void Pawn::ProcessJump() {
	if (m_state == State::JumpLoop || m_state == State::DoubleJumpLoop || m_state == State::TripleJumpLoop ) {
		m_pos.y -= m_jumpSpeed;
		//m_jumpSpeed -= 0.11f;
		m_jumpSpeed *= 0.9f;

		if (m_jumpSpeed <= 1 ) {
			if (m_state == State::JumpLoop) {
				m_state = State::JumpEnd;
			}
			else if (m_state == State::DoubleJumpLoop) {
				m_state = State::DoubleJumpEnd;
			}
			else if (m_state == State::TripleJumpLoop) {
				m_state = State::TripleJumpEnd;
			}

			ResetFallSpeed();
			ResetJumpSpeed();
		}
	}
}

void Pawn::ResetFallSpeed() {
	m_fallSpeed = 1.5;
}

void Pawn::ResetJumpSpeed() {
	//m_jumpSpeed = 6.0;
	m_jumpSpeed = 11.0;

}

void Pawn::ResetBaseSpeed() {
	m_speed = m_baseSpeed;
}

void Pawn::ResetCombo() {
	m_combo = 0;
	m_comboStandard = m_totalDistance;

	ResetBaseSpeed();
	ResetJumpSpeed();
	ResetFallSpeed();
}

void Pawn::NetworkDrawCharacter(HDC hdc, float playerDisX, float thisDisX, float thisY, int cImageIndex, State state) {

	if (thisDisX - playerDisX > 1100)
		return;
	else if (playerDisX - thisDisX > 400)
		return;

	float newPosX = playerDisX - thisDisX;

	m_pos.x = INIT_PAWN_POS_X - newPosX;
	m_pos.y = thisY;
	m_unit->SetImageCount(cImageIndex);

	Draw(hdc, state);
}

void Pawn::FaintCountUp(bool init)
{
	if (init)
	{
		ResetCombo();
	}
	SpeedUpCountDown();
	m_state = State::Stun;
	m_bufSpeed = 0.0f;
	if (m_stackFaint < 0) m_stackFaint = 1;
	else m_stackFaint++;
}

void Pawn::FaintCountDown()
{
	m_stackFaint--;
	if (m_stackFaint <= 0)
	{
		m_bufSpeed = 1.0f;
		m_state = State::JumpEnd;
	}
}

void Pawn::FaintReset()
{
	m_stackFaint = 0;
	m_bufSpeed = 1.0f;
	m_state = State::JumpEnd;
}

void Pawn::SpeedUpCountUp(bool boost)
{
	if (m_stackSpeedUp < 0) m_stackSpeedUp = 1;
	else m_stackSpeedUp++;
	if (boost)
	{
		m_bufSpeed = SPEED_BUFF_VAL_B;
	}
	else
	{
		m_bufSpeed = SPEED_BUFF_VAL;
	}
}

void Pawn::SpeedUpCountDown()
{
	m_stackSpeedUp--;
	if (m_stackSpeedUp <= 0)
	{
		m_bufSpeed = 1.0f;
		m_state = State::JumpEnd;
	}
}