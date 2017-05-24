#pragma once

class IAsynchronousCommand
{
public:
	virtual void execute() = 0;
};
