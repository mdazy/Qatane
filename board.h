#pragma once

#include <QtGui/QColor>

#include <vector>

#include "utils.h"

class Hex {
public:
	enum Type { Invalid = -1, Brick, Wood, Wheat, Sheep, Rock, Desert, Water, nbTypes, Any };
   
	Hex( Type type = Invalid );

public:
	Type type_;
	int number_;
};


/**/


class Node {
public:
	enum Type { None = 0, Town, City };
	Node();

public:
	int player_;
	Type type_;
	Hex::Type harborType_;
};


/**/


class Road {
public:
	Road();

public:
	int player_;
	Pos from_;
	Pos to_;
};


/**/


class Board {
public:
	Board();

	unsigned int hexHeight() const;
	unsigned int hexWidth() const;

	unsigned int nodeHeight() const;
	unsigned int nodeWidth() const;

	unsigned int minHeight() const;
	unsigned int maxHeight() const;

	static std::vector<Pos> nodesAroundHex( const Pos& h );
	static std::vector<Pos> hexesAroundNode( const Pos& n );

public:
	std::vector<std::vector<Hex>> hex_;
	std::vector<std::vector<Node>> node_;
	std::vector<Road> road_;
};