#include "board.h"

using namespace std;


/**/


const QString Hex::typeName[] = { "brick", "wood", "wheat", "sheep", "rock" };


Hex::Hex( Type type ) : type_( type ), number_( -1 ) {
}


ostream& operator <<( ostream& out, const Hex& h ) {
	out << h.type_ << " " << h.number_;
	return out;
}


istream& operator >>( istream& in, Hex& h ) {
	int type;
	in >> type >> h.number_;
	h.type_ = Hex::Type( type );
	return in;
}


/**/


Node::Node() : player_( -1 ), type_( None ), harborType_( Hex::Invalid ) {
}


ostream& operator <<( ostream& out, const Node& n ) {
	out << n.player_ << " " << n.type_ << " " << n.harborType_;
	return out;
}


istream& operator >>( istream& in, Node& n ) {
	int type, harborType;
	in >> n.player_ >> type >> harborType;
	n.type_ = Node::Type( type );
	n.harborType_ = Hex::Type( harborType );
	return in;
}


/**/


Road::Road( int player, const Pos& from, const Pos& to ) : player_( player ) {
	if( to < from ) {
		from_ = to;
		to_ = from;
	} else {
		from_ = from;
		to_ = to;
	}
}


bool Road::operator ==( const Road& rhs ) const {
	return ( player_ == rhs.player_ || player_ < 0 || rhs.player_ < 0 ) && from_ == rhs.from_ && to_ == rhs.to_;
}


ostream& operator <<( ostream& out, const Road& r ) {
	out << r.player_ << " " << r.from_ << " " << r.to_;
	return out;
}


istream& operator >>( istream& in, Road& r ) {
	in >> r.player_ >> r.from_ >> r.to_;
	return in;
}


/**/


/*

The board is composed of dual 2D grids of hexes and nodes.
Hex 0,0 is located at the top left. Hex X and Y coordinates are positive along the SE and S axes, respectively.
Node 0,0 is the NW node of the 0,0 hex. Node X and Y coordinates are positive along horizontal and vertical pseudo-axes.
Note that according to this numbering, some nodes will not correspond to any hex of the hex grid and are therefore invalid.

    +---+                         0,0---1,0
   /     \                         /     \
  /       \                       /       \
 +   0,0   +---+                0,1      1,1---2,1
  \       /     \                 \       /     \
   \     /       \                 \     /       \
    +---+   1,0   +               0,2---1,2       2,2
   /     \       /                 /     \       /
  /       \     /                 /       \     / 
 +   1,1   +---+   \            0,3      1,3---2,3
  \       /     \   x+            \       /     \
   \     /       \                 \     /       \
    +---+   1,1   +               0,4---1,4       2,4
         \       /                       \       / 
      |   \     /                         \     /
      y+   +---+                         1,5---2,5


This yields the following hex-to-node and node-to-hex mappings.

Numbering of nodes around hex x,y   Numbering of hexes around node x,y   Numbering of hexes around node x,y
                                      when x and y have same parity      when x and y have different parity

       x,2y+x---x+1,2y+x                          \                                           /
           /     \                                 \  x,(y-x-2)/2             x-1,(y-x-1)/2  /
          /       \                                 \                                       /
    x,2y+x1  x,y   x+1,2y+x1          x-1,(y-x)/2   x,y----                          ----x,y   x,(y-x-1)/2
          \       /                                 /                                       \
           \     /                                 /  x,(y-x)/2               x-1,(y-x+1)/2  \
     x,2y+x+2---x+1,2y+x+2                        /                                           \


Roads are represented by pairs of start and end nodes.
*/        

Board::Board() {
	// standard map is 7 by 7
	hex_.resize( 7, vector<Hex>( 7 ) );
	node_.resize( hexHeight() * 2 + hexWidth() + 2, vector<Node>( hexWidth() + 1 ) );

	// standard repartition of hexes
	vector<Hex::Type> types = {
		Hex::Wheat, Hex::Wheat, Hex::Wheat, Hex::Wheat,
		Hex::Sheep, Hex::Sheep, Hex::Sheep, Hex::Sheep,
		Hex::Wood, Hex::Wood, Hex::Wood, Hex::Wood,
		Hex::Rock, Hex::Rock, Hex::Rock,
		Hex::Brick, Hex::Brick, Hex::Brick,
		Hex::Desert
	};
	randomize( types );

	// standard numbering of land hexes
	vector<int> numbers = { 5, 2, 6, 3, 8, 10, 9, 12, 11, 4, 8, 10, 9, 4, 5, 6, 3, 11 };

	// map setup
	vector<Pos> hexSetup = {
		// land hexes, ordered along inward spiral (counter clockwise, starting at top)
		// for placement of numbers according to rules
		{ 3, 1 }, { 2, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 2, 5 },
		{ 3, 5 }, { 4, 4 }, { 5, 3 }, { 5, 2 }, { 5, 1 }, { 4, 1 },
		{ 3, 2 }, { 2, 3 }, { 2, 4 }, { 3, 4 }, { 4, 3 }, { 4, 2 }, { 3, 3 },
		// water hexes
		{ 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 1, 2 }, { 1, 6 },
		{ 2, 1 }, { 2, 6 }, { 3, 0 }, { 3, 6 }, { 4, 0 }, { 4, 5 },
		{ 5, 0 }, { 5, 4 }, { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }
	};
	unsigned int iType = 0;
	unsigned int iNumber = 0;
	for( const auto& s : hexSetup ) {
		auto& h = hex_[ s.second ][ s.first ];
		h.type_ = iType < types.size() ? types[ iType++ ] : Hex::Water;
		h.number_ = h.type_ != Hex::Desert && h.type_ != Hex::Water ? numbers[ iNumber++ ] : -1;
		if( h.type_ == Hex::Desert ) {
			robber_ = s;
		}
	}

	// standard repartition of harbors
	vector<Hex::Type> harborTypes = {
		Hex::Wood, Hex::Wheat, Hex::Brick, Hex::Sheep, Hex::Rock, Hex::Any, Hex::Any, Hex::Any, Hex::Any
	};
	randomize( harborTypes );

	vector<pair<pair<int, int>, pair<int, int>>> harborSetup = {
		{ { 3, 5 }, { 4, 5 } }, // NW, NE nodes of hex 3,1
		{ { 2, 6 }, { 2, 7 } }, // NW, W nodes of hex 2, 2
		{ { 1, 9 }, { 1, 10 } }, // NW, W nodes of hex 1, 4
		{ { 1, 12 }, { 1, 13 } }, // W, SW nodes of hex 1, 5
		{ { 2, 14 }, { 3, 14 } }, // SW, SE nodes of hex 2, 5
		{ { 4, 14 }, { 5, 14 } }, // SW, SW nodes of hex 4, 4
		{ { 6, 13 }, { 6, 12 } }, // SE, E nodes of hex 5, 3
		{ { 6, 10 }, { 6, 9 } }, // E, NE nodes of hex 5, 2
		{ { 5, 7 }, { 5, 6 } }, // E, NE nodes of hex 4, 1
	};
	iType = 0;
	for( const auto& s : harborSetup ) {
		auto& n1 = node_[ s.first.second ][ s.first.first ];
		auto& n2 = node_[ s.second.second ][ s.second.first ];
		n1.harborType_ = harborTypes[ iType ];
		n2.harborType_ = harborTypes[ iType++ ];
	}
}


// height in number of hexes
int Board::hexHeight() const {
	return hex_.size();
}


// width in number of hexes
int Board::hexWidth() const {
	if( hexHeight() == 0 ) {
		return 0;
	}
	return hex_[ 0 ].size();
}


// height in number of nodes
int Board::nodeHeight() const {
	return node_.size();
}


// width in number of nodes
int Board::nodeWidth() const {
	if( nodeHeight() == 0 ) {
		return 0;
	}
	return node_[ 0 ].size();
}


// maximum vertical height of the lowest valid hex
int Board::maxHeight() const {
	int result = 0;
	for( int hy = 0; hy < hexHeight(); hy++ ) {
		for( int hx = 0; hx < hexWidth(); hx++ ) {
			if( hex_[ hy ][ hx ].type_ != Hex::Invalid ) {
				result = max( result, 1 + 2 * hy + hx );
			}
		}
	}
	return result;
}

// minimum vertical height of the highest valid hex
int Board::minHeight() const {
	int result = 1 + 2 * hexHeight() + hexWidth();
	for( int hy = 0; hy < hexHeight(); hy++ ) {
		for( int hx = 0; hx < hexWidth(); hx++ ) {
			if( hex_[ hy ][ hx ].type_ != Hex::Invalid ) {
				result = min( result, 1 + 2 * hy + hx );
			}
		}
	}
	return result;
}

// nodes around hex, may include out of bounds indices
vector<Pos> Board::nodesAroundHex( const Pos& h ) {
	vector<Pos> result;
	result.emplace_back( h.x(), 2 * h.y() + h.x() );
	result.emplace_back( h.x(), 2 * h.y() + h.x() + 1);
	result.emplace_back( h.x(), 2 * h.y() + h.x() + 2 );
	result.emplace_back( h.x() + 1, 2 * h.y() + h.x() );
	result.emplace_back( h.x() + 1 , 2 * h.y() + h.x() + 1);
	result.emplace_back( h.x() + 1, 2 * h.y() + h.x() + 2 );
	return result;
}


// nodes around node, may include out of bounds indices
vector<Pos> Board::nodesAroundNode( const Pos& n ) {
	vector<Pos> result;
	result.emplace_back( n.x(), n.y() - 1 );
	result.emplace_back( n.x(), n.y() + 1 );
	if( n.x() % 2 == n.y() % 2 ) {
		result.emplace_back( n.x() + 1, n.y() );
	} else {
		result.emplace_back( n.x() - 1, n.y() );
	}
	return result;	
}


// hexes around node, may include out of bounds indices
vector<Pos> Board::hexesAroundNode( const Pos& n ) {
	vector<Pos> result;
	if( n.x() % 2 == n.y() % 2 ) {
		result.emplace_back( n.x() - 1, ( n.y() - n.x() ) / 2 );
		result.emplace_back( n.x(), ( n.y() - n.x() - 2 ) / 2 );
		result.emplace_back( n.x(), ( n.y() - n.x() ) / 2 );
	} else {
		result.emplace_back( n.x() - 1, ( n.y() - n.x() - 1 ) / 2 );
		result.emplace_back( n.x(), ( n.y() - n.x() - 1 ) / 2 );
		result.emplace_back( n.x() - 1, ( n.y() - n.x() + 1 ) / 2 );
	}
	return result;

}


// return true if at least one hex around the node is not water
bool Board::landNode( const Pos& n ) const {
	for( const auto& h : hexesAroundNode( n ) ) {
		if( !h.valid() || h.x() >= hexWidth() || h.y() >= hexHeight() ) {
			continue;
		}
		if( hex_[ h.y() ][ h.x() ].type_ > Hex::Invalid && hex_[ h.y() ][ h.x() ].type_ < Hex::Water ) {
			return true;
		}
	}
	return false;
}


bool Board::roadExists( const Pos& from, const Pos& to, int player ) const {
	return find( road_.begin(), road_.end(), Road( player, from, to ) ) != road_.end();
}


int Board::longestRoad( const Pos& from, int p, int visit ) const {
	int length = 0;
	for( const auto& to : nodesAroundNode( from ) ) {
		auto roadIt = find( road_.begin(), road_.end(), Road( p, from, to ) );
		if( roadIt == road_.end() ) {
			// no road for this player
			continue;
		}
		if( roadIt->visit_ == visit ) {
			// road already visited
			continue;
		}
		// mark road as visited
		roadIt->visit_ = visit;
		// inspect neighbors at end point
		const auto& n = node_[ to.y() ][ to.x() ];
		if( n.type_ != Node::None && n.player_ != p ) {
			// road cut by opponent construction
			length = max( length, 1 );
		} else {
			length = max( length, 1 + longestRoad( to, p, visit ) );
		}
		// unmark road
		roadIt->visit_ = visit - 1;
	}
	return length;
}


int Board::longestRoadForPlayer( int p ) const {
	static int visit = 0;

	int length = 0;
	for( const auto& r : road_ ) {
		if( r.player_ != p ) {
			continue;
		}
		visit++;
		int l = longestRoad( r.from_, p, visit );
		length = max( length, l );
		l = longestRoad( r.to_, p, visit );
		length = max( length, l );
	}
	return length;
}


ostream& operator <<( ostream& out, const Board& b ) {
	out << "# Board" << endl;
	out << b.hexWidth() << " " << b.hexHeight() << endl;
	out << b.robber_ << endl;

	out << "# Hexes" << endl;
	for( int hx = 0; hx < b.hexWidth(); hx++ ) {
		for( int hy = 0; hy < b.hexHeight(); hy++ ) {
			out << b.hex_[ hy ][ hx ] << " ";
		}
		out << endl;
	}

	out << "# Nodes" << endl;
	for( int nx = 0; nx < b.nodeWidth(); nx++ ) {
		for( int ny = 0; ny < b.nodeHeight(); ny++ ) {
			out << b.node_[ ny ][ nx ] << " ";
		}
		out << endl;
	}

	out << "# Roads" << endl;
	out << b.road_.size() << endl;
	for( const auto& r : b.road_ ) {
		out << r << endl;
	}

	return out;
}


istream& operator >>( istream& in, Board& b ) {
	string dummy;

	getline( in, dummy );
	int hexWidth, hexHeight;
	in >> hexWidth >> hexHeight; in.ignore();
	in >> b.robber_; in.ignore();
	b.hex_.resize( hexHeight, vector<Hex>( hexWidth ) );
	b.node_.resize( hexHeight * 2 + hexWidth + 2, vector<Node>( hexWidth + 1 ) );

	getline( in, dummy );
	for( int hx = 0; hx < b.hexWidth(); hx++ ) {
		for( int hy = 0; hy < b.hexHeight(); hy++ ) {
			in >> b.hex_[ hy ][ hx ];
		}
		in.ignore();
	}
	in.ignore();

	getline( in, dummy );
	for( int nx = 0; nx < b.nodeWidth(); nx++ ) {
		for( int ny = 0; ny < b.nodeHeight(); ny++ ) {
			in >> b.node_[ ny ][ nx ];
		}
		in.ignore();
	}
	in.ignore();

	getline( in, dummy );
	int nbRoads;
	in >> nbRoads; in.ignore();
	b.road_.resize( nbRoads );
	for( int i = 0; i < nbRoads; i++ ) {
		in >> b.road_[ i ]; in.ignore();
	}
	
	return in;
}

