#ifndef CQQuinto_H
#define CQQuinto_H

#include <QFrame>
#include <set>
#include <cassert>

class CQQuintoTileSet;
class CQQuintoPlayer;
class CQQuintoTurn;
class CQQuintoBoard;
class CQQuintoCell;
class CQQuintoTile;
class CQQuintoMove;

class QToolButton;
class QPushButton;

class CQQuinto : public QFrame {
  Q_OBJECT

  Q_PROPERTY(bool   showComputer       READ isShowComputer     WRITE setShowComputer      )
  Q_PROPERTY(QColor invalidTileColor   READ invalidTileColor   WRITE setInvalidTileColor  )
  Q_PROPERTY(QColor currentTileColor   READ currentTileColor   WRITE setCurrentTileColor  )
  Q_PROPERTY(QColor player1TileColor   READ player1TileColor   WRITE setPlayer1TileColor  )
  Q_PROPERTY(QColor player2TileColor   READ player2TileColor   WRITE setPlayer2TileColor  )
  Q_PROPERTY(QColor validMoveColor     READ validMoveColor     WRITE setValidMoveColor    )
  Q_PROPERTY(QColor currentPlayerColor READ currentPlayerColor WRITE setCurrentPlayerColor)

 public:
  enum class TileOwner {
    NONE,
    PLAYER1,
    PLAYER2,
    BOARD,
    TILE_SET
  };

  enum class Direction {
    NONE,
    HORIZONTAL,
    VERTICAL
  };

  enum class Side {
    NONE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
  };

  struct TilePosition {
    int ix;
    int iy;

    TilePosition(int ix, int iy) :
     ix(ix), iy(iy) {
    }

    bool operator==(const TilePosition &rhs) const {
      return (ix == rhs.ix && iy == rhs.iy);
    }

    bool operator<(const TilePosition &rhs) const {
      return (ix < rhs.ix || (ix == rhs.ix && iy < rhs.iy));
    }
  };

  using TilePositions = std::set<TilePosition>;

  struct TileData {
    TileOwner owner { TileOwner::NONE };
    int       x     { 0 };
    int       y     { 0 };

    TileData() = default;

    TileData(TileOwner owner, int x, int y) :
     owner(owner), x(x), y(y) {
    }

    TileData(TileOwner owner, const TilePosition &pos) :
     owner(owner), x(pos.ix), y(pos.iy) {
    }

    void print(std::ostream &os) const {
      if      (owner == TileOwner::PLAYER1 ) { os << "Player1:" << x; }
      else if (owner == TileOwner::PLAYER2 ) { os << "Player2:" << x; }
      else if (owner == TileOwner::BOARD   ) { os << "Board:" << x << "," << y; }
      else if (owner == TileOwner::TILE_SET) { os << "TileSet:"; }
    }
  };

 public:
  CQQuinto(QWidget *parent = 0);
 ~CQQuinto();

  int nx() const { return 18; }
  int ny() const { return 12; }

  int handSize() const { return 5; }

  CQQuintoTile *getTile();

  CQQuintoPlayer *player1() const { return player1_; }
  CQQuintoPlayer *player2() const { return player2_; }

  CQQuintoPlayer *currentPlayer() const { return currentPlayer_; }

  CQQuintoPlayer *player(TileOwner owner) {
    return (owner == TileOwner::PLAYER1 ? player1() : player2());
  }

  CQQuintoTurn *turn() const { return turn_; }

  //---

  bool isShowComputer() const { return showComputer_; }
  void setShowComputer(bool b) { showComputer_ = b; }

  const QColor &invalidTileColor() const { return invalidTileColor_; }
  void setInvalidTileColor(const QColor &v) { invalidTileColor_ = v; }

  const QColor &currentTileColor() const { return currentTileColor_; }
  void setCurrentTileColor(const QColor &v) { currentTileColor_ = v; }

  const QColor &player1TileColor() const { return player1TileColor_; }
  void setPlayer1TileColor(const QColor &v) { player1TileColor_ = v; }

  const QColor &player2TileColor() const { return player2TileColor_; }
  void setPlayer2TileColor(const QColor &v) { player2TileColor_ = v; }

  const QColor &validMoveColor() const { return validMoveColor_; }
  void setValidMoveColor(const QColor &v) { validMoveColor_ = v; }

  const QColor &currentPlayerColor() const { return currentPlayerColor_; }
  void setCurrentPlayerColor(const QColor &v) { currentPlayerColor_ = v; }

  //---

  void updateState();

  void updateFonts();

  void showBestMove() const;

  void addMove(const CQQuintoMove &move);

  void undoMove(const CQQuintoMove &move);

  void doMove(const CQQuintoMove &move);
  void doMove(const CQQuinto::TileData &from, const CQQuinto::TileData &to);

  void computerMove();

  void nextTurn();

  void gameOver();

  bool canMove() const;

  bool isTurnValid(int &score) const;

  void apply();

  void newGame();

  int moveScore(const CQQuintoMove &move) const;

  void resizeEvent(QResizeEvent *);

  double calcFontScale(double s) const;

  QSize sizeHint() const;

 private slots:
  void cancelSlot();
  void backSlot();
  void applySlot();
  void newGameSlot();

 private:
  void updateWidgets();

 private:
  using Turns = std::vector<CQQuintoTurn *>;

  CQQuintoTileSet* tileSet_       { nullptr };
  CQQuintoPlayer*  player1_       { nullptr };
  CQQuintoPlayer*  player2_       { nullptr };
  CQQuintoPlayer*  currentPlayer_ { nullptr };
  CQQuintoBoard*   board_         { nullptr };
  CQQuintoTurn*    turn_          { nullptr };
  Turns            turns_;

  bool showComputer_ { false };

  QColor invalidTileColor_   { "#aa4444" };
  QColor currentTileColor_   { "#aaaa44" };
  QColor player1TileColor_   { "#888866" };
  QColor player2TileColor_   { "#886688" };
  QColor validMoveColor_     { "#add4ab" };
  QColor currentPlayerColor_ { "#64d444" };

  QToolButton* cancelButton_  { nullptr };
  QToolButton* backButton_    { nullptr };
  QToolButton* applyButton_   { nullptr };
  QPushButton* newGameButton_ { nullptr };

  double lastFs_ { 1 };

  bool gameOver_ { false };
};

//---

class CQQuintoMove {
 public:
  CQQuintoMove() = default;

  CQQuintoMove(const CQQuinto::TileData &from, const CQQuinto::TileData &to) :
   from_(from), to_(to) {
  }

  const CQQuinto::TileData &from() const { return from_; }
  const CQQuinto::TileData &to  () const { return to_  ; }

  bool isValid() const {
    return (from_.owner != CQQuinto::TileOwner::NONE && to_.owner != CQQuinto::TileOwner::NONE);
  }

  void print(std::ostream &os) const {
    os << "From: "; from_.print(os); os << " To: "  ; to_.print(os);
  }

 private:
  CQQuinto::TileData from_;
  CQQuinto::TileData to_;
};

//---

class CQQuintoBoard : public QWidget {
  Q_OBJECT

 public:
  using Direction     = CQQuinto::Direction;
  using Side          = CQQuinto::Side;
  using TilePosition  = CQQuinto::TilePosition;
  using TilePositions = CQQuinto::TilePositions;

  struct TileLine {
    Direction direction { Direction::NONE };
    int       start     { -1 };
    int       end       { -1 };
    int       pos       { -1 };
    int       sum       { 0 };
    int       current   { 0 };

    TileLine() = default;

    TileLine(Direction direction, int start, int end, int pos) :
     direction(direction), start(start), end(end), pos(pos) {
    }

    int len() const { assert(start >= 0 && end >= start); return end - start + 1; }

    bool hasPosition(const TilePosition &p) const {
      if (direction == Direction::HORIZONTAL)
        return (p.iy == pos && p.ix >= start && p.ix <= end);
      else
        return (p.ix == pos && p.iy >= start && p.iy <= end);
    }

    TilePosition startPos() const {
      if (direction == Direction::HORIZONTAL)
        return TilePosition(start, pos);
      else
        return TilePosition(pos, start);
    }

    bool isValid(bool &partial, QString &errMsg) const {
      if (len() > 5) {
        errMsg = "Line too long";
        return false;
      }

      //---

      if ((sum % 5) != 0) {
        if (len() == 5) {
          errMsg = "Not a multiple of 5";
          return false;
        }

        partial = true;
        errMsg  = "Not a multiple of 5 (yet)";
      }

      return true;
    }

    void print(std::ostream &os) const {
      if (direction == Direction::HORIZONTAL)
        os << "H: (" << start << "," << pos << ") (" << end << ") = " << sum;
      else
        os << "V: (" << pos << "," << start << ") (" << end << ") = " << sum;
    }

    void print(std::ostream &os, const QString &errMsg) const {
      print(os);

      if (errMsg != "")
        os << " (" << errMsg.toStdString() << ")";
    }
  };

  using TileLines = std::vector<TileLine>;

  struct BoardDetails {
    bool          valid   { false };
    bool          partial { false };
    int           score   { 0 };
    TileLines     lines;
    TilePositions validPositions;
    QString       errMsg;

    void reset() {
      valid = false;
      score = 0;

      lines         .clear();
      validPositions.clear();
    }
  };

  using Moves = std::vector<CQQuintoMove>;

  using MovesList = std::vector<Moves>;

  struct MoveTree {
    using Children  = std::vector<MoveTree *>;
    using MoveTrees = std::vector<const MoveTree *>;
    using ScoreTree = std::map<int,MoveTrees>;

    MoveTree*    parent  { nullptr };
    CQQuintoMove move;
    Children     children;
    bool         partial { false };
    int          score   { 0 };

    MoveTree() { }

   ~MoveTree() {
      for (auto &child : children)
        delete child;
    }

    void updateScoreTree(ScoreTree &scoreTree) const {
      if (! partial)
        scoreTree[-score].push_back(this);

      if (! children.empty()) {
        for (const auto &child : children)
          child->updateScoreTree(scoreTree);
      }
    }

    const MoveTree *maxLeaf() const {
      ScoreTree scoreTree;

      updateScoreTree(scoreTree);

      if (scoreTree.empty())
        return nullptr;

      const MoveTrees &moveTrees = scoreTree.begin()->second;

      assert(! moveTrees.empty());

      int nt = moveTrees.size();

      const MoveTree *minTree = moveTrees[0];

      if (nt == 1)
        return minTree;

      int minDepth = minTree->depth();

      for (int i = 1; i < nt; ++i) {
        const MoveTree *tree = moveTrees[i];

        int depth = tree->depth();

        if (depth < minDepth) {
          minDepth = depth;
          minTree  = tree;
        }
      }

      return minTree;
    }

    int depth() const {
      if (! parent)
        return 1;

      return parent->depth() + 1;
    }

    void printDepth(std::ostream &os, int depth) const {
      if (move.isValid()) {
        os << "{";

        move.print(os);

        os << "}";
      }

      if (! children.empty()) {
        os << "\n";

        for (int i = 0; i < depth; ++i)
          os << " ";

        bool first = true;

        os << "[";

        for (const auto &child : children) {
          if (! first)
            os << " ";

          child->printDepth(os, depth + 1);

          first = false;
        }

        os << "]";
      }

      os << "@" << score;
    }

    void print(std::ostream &os) const {
      printDepth(os, 0);
    }
  };

 public:
  CQQuintoBoard(CQQuinto *quinto);

 ~CQQuintoBoard();

  CQQuintoCell *cell(int x, int y) { return cells_[y][x]; }

  CQQuintoTile *cellTile(int x, int y) const;

  CQQuintoTile *takeCellTile(int x, int y);

  void playBestMove();
  void showBestMove() const;

  bool getBestMove(Moves &moves, int &score) const;

  MoveTree *boardMoveTree() const;

  bool boardMoves(Moves &moves, int &score, bool &partial) const;

  void boardDetails(BoardDetails &details) const;

  void getBoardLines(TileLines &hlines, TileLines &vlines) const;

  void drawBoard(QPainter *painter);

  double boardTileSize() const { return bs_; }
  double playerTileSize() const { return ps_; }

 private:
  using TileOwner = CQQuinto::TileOwner;
  using TileData  = CQQuinto::TileData;

  MoveTree *buildBoardMoveTree(int depth) const;

  int countTiles(int ix, int iy, Side side) const;

  TileData posToTileData(const QPoint &pos) const;

  void paintEvent(QPaintEvent *);

  void drawPlayerTiles(QPainter *painter, CQQuintoPlayer *player, Qt::Alignment align);

  void drawTurn(QPainter *painter);

  void drawScores(QPainter *painter);

  void drawTile(QPainter *painter, CQQuintoTile *tile, const QRectF &rect,
                double s, const QBrush &brush);

  void mousePressEvent  (QMouseEvent *);
  void mouseMoveEvent   (QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);

  void keyPressEvent(QKeyEvent *);

 private:
  using ColCells    = std::vector<CQQuintoCell *>;
  using RowColCells = std::vector<ColCells>;

  CQQuinto*     quinto_ { nullptr };
  double        x_      { 0.0 };
  double        y_      { 0.0 };
  double        bs_     { 1.0 };
  double        ps_     { 1.0 };
  double        py_     { 0.0 };
  RowColCells   cells_;
  TileData      pressData_;
  TileData      releaseData_;
  QPoint        dragPos_;
  CQQuintoTile* dragTile_ { nullptr };
  BoardDetails  details_;
};

//---

class CQQuintoTile : public QWidget {
  Q_OBJECT

 public:
  using Owner = CQQuinto::TileOwner;

 public:
  CQQuintoTile(CQQuinto *quinto, int ind, int value);

  int value() const { return value_; }

  const Owner &owner() const { return owner_; }
  void setOwner(const Owner &v) { owner_ = v; }

  const Owner &player() const { return player_; }
  void setPlayer(const Owner &v) { player_ = v; }

  int turn() const { return turn_; }
  void setTurn(int i) { turn_ = i; }

  void show(const QRect &rect);
  void hide();

  void setSize(double s);

  double fontScale() const { return fs_; }
  void setFontScale(double fs) { fs_ = fs; }

  void drawTile(QPainter *painter);
  void drawTile(QPainter *painter, const QRectF &rect);

 private:
  void paintEvent(QPaintEvent *);

 private:
  CQQuinto* quinto_ { nullptr };
  int       ind_    { -1 };
  int       value_  { -1 };
  Owner     owner_  { Owner::NONE };
  Owner     player_ { Owner::NONE };
  int       turn_   { -1 };
  double    s_      { 1 };
  double    fs_     { 1 };
  QFont     font_;
};

//----

class CQQuintoTileSet {
 public:
  CQQuintoTileSet(CQQuinto *quinto);
 ~CQQuintoTileSet();

  void shuffle();

  CQQuintoTile *getTile();

  void ungetTile(CQQuintoTile *tile);

 private:
  using Tiles = std::vector<CQQuintoTile *>;

  CQQuinto* quinto_ { nullptr };
  Tiles     tiles_;
  Tiles     itiles_;
};

//----

class CQQuintoPlayer {
 public:
  using Tiles = std::vector<CQQuintoTile *>;

 public:
  enum class Type {
    NONE,
    HUMAN,
    COMPUTER
  };

 public:
  CQQuintoPlayer(CQQuinto *quinto, int ind, Type type);

  int ind() const { return ind_; }

  Type type() const { return type_; }

  const QString &name() const { return name_; }
  void setName(const QString &s) { name_ = s; }

  const Tiles &tiles() const { return tiles_; }

  int numTiles() const;

  CQQuintoTile *tile(int i) const;

  CQQuintoTile *takeTile(int i, bool nocheck=false);

  void addTile(CQQuintoTile *tile, int i);

  int score() const { return score_; }
  void setScore(int i) { score_ = i; }

  int tileX() const { return tileX_; }
  void setTileX(int i) { tileX_ = i; }

  int tileY() const { return tileY_; }
  void setTileY(int i) { tileY_ = i; }

  void addScore(int i) { score_ += i; }

  CQQuinto::TileOwner owner() const;

  void drawTiles();

 private:
  CQQuinto* quinto_ { nullptr };
  QString   name_;
  int       ind_    { -1 };
  Type      type_   { Type::NONE };
  Tiles     tiles_;
  int       score_  { 0 };
  int       tileX_  { 0 };
  int       tileY_  { 0 };
};

//----

class CQQuintoCell {
 public:
  CQQuintoCell(int x, int y) :
   x_(x), y_(y) {
  }

  CQQuintoTile *tile() const { return tile_; }
  void setTile(CQQuintoTile *tile) { tile_ = tile; }

 private:
  int           x_    { 0 };
  int           y_    { 0 };
  CQQuintoTile* tile_ { nullptr };
};

//----

class CQQuintoTurn {
 public:
  using Moves = std::vector<CQQuintoMove>;

 public:
  CQQuintoTurn(CQQuinto *quinto, int ind);

  int ind() const { return ind_; }

  const Moves &moves() const { return moves_; }

  const CQQuintoMove &move(int i) { return moves_[i]; }

  void addMove(const CQQuintoMove &move) { moves_.push_back(move); }

  void popMove() { moves_.pop_back(); }

  void clear() { moves_.clear(); }

  int score() const;

 private:
  CQQuinto *quinto_ { nullptr };
  int       ind_    { -1 };
  Moves     moves_;
};

#endif
