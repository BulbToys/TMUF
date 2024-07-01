#include <cstring>

#include "modules.h"

Module::Module(const char* name, Module::InitFn* Init, Module::PanelFn* Panel, Module::EndFn* End)
{
	this->name = name;
	this->Init = Init;
	this->Panel = Panel;
	this->End = End;
	this->next = nullptr;
	this->prev = nullptr;

	if (!first)
	{
		Module::first = this;
		Module::last = this;
	}
	else
	{
		if (strcmp(first->name, name) > 0)
		{
			Module::first->prev = this;
			this->next = Module::first;
			Module::first = this;

			return;
		}

		auto iter = Module::first;
		while (iter->next)
		{
			if (strcmp(iter->next->name, name) > 0)
			{
				this->next = iter->next;
				this->prev = iter;

				iter->next = this;

				return;
			}

			iter = iter->next;
		}

		this->prev = iter;
		iter->next = this;
		Module::last = this;
	}
}

void Modules::Init()
{
	auto iter = Module::First();
	while (iter)
	{
		auto InitFunc = iter->InitFunc();
		if (InitFunc)
		{
			InitFunc();
		}
		iter = iter->Next();
	}
}

std::vector<IPanel*> Modules::Panels(Module::DrawType dt)
{
	std::vector<IPanel*> panels;

	auto iter = Module::First();
	while (iter)
	{
		auto PanelFunc = iter->PanelFunc();
		if (PanelFunc)
		{
			auto panel = PanelFunc(dt);
			if (panel)
			{
				panels.push_back(panel);
			}
		}
		iter = iter->Next();
	}

	return panels;
}

void Modules::End()
{
	auto iter = Module::Last();
	while (iter)
	{
		auto EndFunc = iter->EndFunc();
		if (EndFunc)
		{
			EndFunc();
		}
		iter = iter->Prev();
	}
}