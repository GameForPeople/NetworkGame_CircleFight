#include "Pawn.h"

Pawn::Pawn()
{
}

Pawn::Pawn(CharacterName inputCharacterName)
{
	if (inputCharacterName == CharacterName::Archer)
		m_unit = new CArcher;
	else if (inputCharacterName == CharacterName::Zombie)
		m_unit = new CZombie;

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

void Pawn::Update(State state) {

	ComputeTotalDistance();
	ProcessCombo();
	ProcessGravity();
	ProcessJump();

	m_unit->Update(state);
}
//void Pawn::Draw(HDC hdc, float x, float y, float sizeX, float sizeY) {
//	m_unit->Draw(hdc, m_pos.x, m_pos.y, m_size.x, m_size.y, );
//}

void Pawn::Draw(HDC hdc, State state) {
	m_unit->Draw(hdc, m_pos.x, m_pos.y, m_size.x, m_size.y, state);
}

void Pawn::ComputeTotalDistance() {
	m_totalDistance += m_speed;
}

void Pawn::ProcessCombo() {
	//if (m_state != State::Stun && m_state != State::Collide && m_state != State::Death) {
	//if ((int)m_totalDistance % 100 == 1) {	//�̷��� �ȵ�... 1�پ�������� ����.
	//m_combo = curFlatNum - m_comboStandard;	//����??
	m_combo = (m_totalDistance - m_comboStandard) / 100;

	std::cout << m_combo << "   " << m_speed << "   " << m_baseSpeed << std::endl;

	if (m_combo % 50 == 1)
		m_comboEffect = true;

	if (m_comboEffect) {
		if (m_combo != 0 && m_combo % 50 == 0) {
			m_speed = m_speed + m_baseSpeed / 5;
			m_comboEffect = false;
		}
	}
}

bool Pawn::InsertKey(WPARAM Key) {
	if (Key == VK_SPACE) {
		if (m_state == State::Run) {
			m_state = State::JumpStart;
			ResetJumpSpeed();
			m_unit->SetImageCount(m_unit->GetJumpImageCount());
			m_state = State::JumpLoop;
			return true;
		}	
		else if (m_state == State::JumpLoop || m_state == State::JumpEnd) {
			ResetJumpSpeed();
			m_state = State::DoubleJumpStart;
			ResetFallSpeed();
			m_unit->SetImageCount(m_unit->GetJumpImageCount());
			m_state = State::DoubleJumpLoop;
			return true;
		}		
	}
	return false;
}

void Pawn::ProcessGravity() {
	if (m_state == State::Fall || m_state == State::JumpEnd || m_state == State::DoubleJumpEnd) {
		m_pos.y += m_fallSpeed;
		//m_fallSpeed *= 1.05;
		m_fallSpeed *= 1.1;
	}
}

void Pawn::ProcessJump() {
	if (m_state == State::JumpLoop || m_state == State::DoubleJumpLoop) {
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

void Pawn::NetworkDrawCharacter(HDC hdc, float playerDisX, float thisDisX, float thisY, int cImageIndex, int combo, State state) {

	if (thisDisX - playerDisX > 1100)
		return;
	else if (playerDisX - thisDisX > 300)
		return;

	float newPosX = playerDisX - thisDisX;

	if (newPosX < 0) {
		m_pos.x = INIT_PAWN_POS_X - newPosX;
		m_pos.y = thisY;
		m_unit->SetImageCount(cImageIndex);

		Draw(hdc, state);
	}
}