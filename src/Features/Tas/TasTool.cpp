#include "TasTool.hpp"

#include "TasPlayer.hpp"
#include <algorithm>

std::list<TasTool *> &TasTool::GetList(int slot) {
	static std::list<TasTool *> list[2];
	return list[slot];
}

TasTool::TasTool(const char *name, int slot)
	: name(name)
	, slot(slot) {
	this->GetList(slot).push_back(this);
}

TasTool::~TasTool() {
}

const char *TasTool::GetName() {
	return this->name;
}

void TasTool::SetParams(std::shared_ptr<TasToolParams> params) {
	this->params = params;
	this->updated = true;

	// the tool has been updated. prioritize it by moving it to the end of the global list
	// mlugg please don't kill me
	GetList(slot).splice(GetList(slot).end(), GetList(slot), std::find(GetList(slot).begin(), GetList(slot).end(), this));
}

void TasTool::Reset() {
	params = std::make_shared<TasToolParams>();
}

std::shared_ptr<TasToolParams> TasTool::GetCurrentParams() {
	return params;
}
