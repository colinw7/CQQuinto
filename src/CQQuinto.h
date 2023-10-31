#ifndef CQQuinto_H
#define CQQuinto_H

#include <QFrame>
#include <set>
#include <memory>
#include <cassert>
#include <iostream>

class QToolButton;
class QPushButton;
class QComboBox;

namespace CQQuinto {

class App;
class TileSet;
class Player;
class Turn;
class Board;
class Tile;
class Move;

//------

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

enum class TileOwner {
  NONE,
  PLAYER1,
  PLAYER2,
  BOARD,
  TILE_SET
};

enum class PlayerType {
  NONE,
  HUMAN,
  COMPUTER
};

enum class PlayMode {
  HUMAN_HUMAN,
  HUMAN_COMPUTER,
  COMPUTER_HUMAN,
  COMPUTER_COMPUTER
};

//----

class Player {
 public:
  using Tiles = std::vector<Tile *>;

 public:
  Player(App *quinto, TileOwner owner, const QString &name, PlayerType type);

  TileOwner owner() const { return owner_; }

  PlayerType type() const { return type_; }
  void setType(PlayerType type) { type_ = type; }

  const QString &name() const { return name_; }
  void setName(const QString &s) { name_ = s; }

  const Tiles &tiles() const { return tiles_; }

  int numTiles() const;

  Tile *tile(int i) const;

  Tile *takeTile(int i, bool nocheck=false);

  void addTile(Tile *tile, int i);

  int score() const { return score_; }
  void setScore(int i) { score_ = i; }

  bool canMove() const { return canMove_; }
  void setCanMove(bool b) { canMove_ = b; }

  int tileX() const { return tileX_; }
  void setTileX(int i) { tileX_ = i; }

  int tileY() const { return tileY_; }
  void setTileY(int i) { tileY_ = i; }

  void addScore(int i) { score_ += i; }

  void drawTiles();

 private:
  App*       quinto_  { nullptr };
  TileOwner  owner_   { TileOwner::NONE };
  QString    name_;
  PlayerType type_    { PlayerType::NONE };
  Tiles      tiles_;
  int        score_   { 0 };
  bool       canMove_ { true };
  int        tileX_   { 0 };
  int        tileY_   { 0 };
};

using PlayerP = std::unique_ptr<Player>;

//------

class TileSet {
 public:
  TileSet(App *quinto);
 ~TileSet();

  void shuffle();

  Tile *getTile();

  void ungetTile(Tile *tile);

 private:
  using Tiles = std::vector<Tile *>;

  App*  quinto_ { nullptr };
  Tiles tiles_;
  Tiles itiles_;
};

using TileSetP = std::unique_ptr<TileSet>;

//------

struct TilePosition {
  int ix { -1 };
  int iy { -1 };

  TilePosition() = default;

  TilePosition(int ix, int iy) :
   ix(ix), iy(iy) {
  }

  bool isValid() const { return (ix >= 0 && iy >= 0); }

  TilePosition left  () const { return TilePosition(ix - 1, iy    ); }
  TilePosition right () const { return TilePosition(ix + 1, iy    ); }
  TilePosition top   () const { return TilePosition(ix    , iy - 1); }
  TilePosition bottom() const { return TilePosition(ix    , iy + 1); }

  bool operator==(const TilePosition &rhs) const {
    return (ix == rhs.ix && iy == rhs.iy);
  }

  bool operator<(const TilePosition &rhs) const {
    return (ix < rhs.ix || (ix == rhs.ix && iy < rhs.iy));
  }
};

using TilePositions = std::set<TilePosition>;

struct TileData {
  TileOwner    owner { TileOwner::NONE };
  TilePosition pos;

  TileData() = default;

  TileData(TileOwner owner, const TilePosition &pos);

  void print(std::ostream &os) const;
};

struct ValidScore {
  bool valid { false };
  int  score { 0 };
};

//------

class App : public QFrame {
  Q_OBJECT

  Q_PROPERTY(PlayMode playMode           READ playMode           WRITE setPlayMode          )
  Q_PROPERTY(QColor   invalidTileColor   READ invalidTileColor   WRITE setInvalidTileColor  )
  Q_PROPERTY(QColor   currentTileColor   READ currentTileColor   WRITE setCurrentTileColor  )
  Q_PROPERTY(QColor   player1TileColor   READ player1TileColor   WRITE setPlayer1TileColor  )
  Q_PROPERTY(QColor   player2TileColor   READ player2TileColor   WRITE setPlayer2TileColor  )
  Q_PROPERTY(QColor   validMoveColor     READ validMoveColor     WRITE setValidMoveColor    )
  Q_PROPERTY(QColor   currentPlayerColor READ currentPlayerColor WRITE setCurrentPlayerColor)
  Q_PROPERTY(QColor   tileBgColor        READ tileBgColor        WRITE setTileBgColor       )
  Q_PROPERTY(QColor   tileBorderColor    READ tileBorderColor    WRITE setTileBorderColor   )

  Q_ENUMS(PlayMode)

 public:
  App(QWidget *parent = nullptr);
 ~App();

  int nx() const { return 18; }
  int ny() const { return 12; }

  int handSize() const { return 5; }

  const TileSetP &tileSet() const { return tileSet_; }

  const PlayerP &player1() const { return player1_; }
  const PlayerP &player2() const { return player2_; }

  TileOwner currentPlayerOwner() const { return currentPlayerOwner_; };

  const PlayerP &currentPlayer() const { return ownerPlayer(currentPlayerOwner()); }

  const PlayerP &ownerPlayer(TileOwner owner) const {
    return (owner == TileOwner::PLAYER1 ? player1() : player2());
  }

  Turn *turn() const { return turn_; }

  //---

  void init();

  void createWidgets();

  //---

  PlayMode playMode() const { return playMode_; }
  void setPlayMode(PlayMode mode);

  const QColor &invalidTileColor() const { return invalidTileColor_; }
  void setInvalidTileColor(const QColor &c) { invalidTileColor_ = c; }

  const QColor &currentTileColor() const { return currentTileColor_; }
  void setCurrentTileColor(const QColor &c) { currentTileColor_ = c; }

  const QColor &player1TileColor() const { return player1TileColor_; }
  void setPlayer1TileColor(const QColor &c) { player1TileColor_ = c; }

  const QColor &player2TileColor() const { return player2TileColor_; }
  void setPlayer2TileColor(const QColor &c) { player2TileColor_ = c; }

  const QColor &validMoveColor() const { return validMoveColor_; }
  void setValidMoveColor(const QColor &c) { validMoveColor_ = c; }

  const QColor &currentPlayerColor() const { return currentPlayerColor_; }
  void setCurrentPlayerColor(const QColor &c) { currentPlayerColor_ = c; }

  const QColor &tileBgColor() const { return tileBgColor_; }
  void setTileBgColor(const QColor &c) { tileBgColor_ = c; }

  const QColor &tileBorderColor() const { return tileBorderColor_; }
  void setTileBorderColor(const QColor &c) { tileBorderColor_ = c; }

  //---

  void updateState();

  void updateFonts();

  void addMove(const Move &move);

  void undoMove(const Move &move);

  void doMove(const Move &move);

  void doMoveParts(const TileData &from, const TileData &to);

  void computerMove();

  void playComputerMove();

  void nextTurn();

  bool isGameOver() const { return gameOver_; }
  void setGameOver(bool b);

  bool canMove() const;

  ValidScore isTurnValid() const;

  void apply(bool next=true);

  void cancel();

  void back();

  void newGame();

  int moveScore(const Move &move) const;

  void resizeEvent(QResizeEvent *) override;

  double calcFontScale(double s) const;

  QSize sizeHint() const override;

 private slots:
  void cancelSlot();
  void backSlot();
  void applySlot();
  void newGameSlot();
  void modeSlot(int);

 private:
  void updateWidgets();

 private:
  using Turns = std::vector<Turn *>;

  TileSetP  tileSet_;
  PlayerP   player1_;
  PlayerP   player2_;
  Board*    board_    { nullptr };
  Turn*     turn_     { nullptr };
  Turns     turns_;

  TileOwner currentPlayerOwner_ { TileOwner::PLAYER1 };

  QColor invalidTileColor_   { "#aa4444" };
  QColor currentTileColor_   { "#7459aa" };
  QColor player1TileColor_   { "#3d4c7f" };
  QColor player2TileColor_   { "#366338" };
  QColor validMoveColor_     { "#cfe5cc" };
  QColor currentPlayerColor_ { "#64d444" };
  QColor tileBgColor_        { "#c0c1a0" };
  QColor tileBorderColor_    { "#000000" };

  QToolButton* cancelButton_  { nullptr };
  QToolButton* backButton_    { nullptr };
  QToolButton* applyButton_   { nullptr };
  QPushButton* newGameButton_ { nullptr };
  QComboBox*   modeCombo_     { nullptr };

  double lastFs_ { 1 };

  bool gameOver_ { false };

  PlayMode playMode_ { PlayMode::HUMAN_COMPUTER };
};

//---

class Move {
 public:
  Move() = default;

  Move(const TileData &from, const TileData &to) :
   from_(from), to_(to) {
  }

  const TileData &from() const { return from_; }
  const TileData &to  () const { return to_  ; }

  bool isValid() const {
    return (from_.owner != TileOwner::NONE && to_.owner != TileOwner::NONE);
  }

  void print(std::ostream &os) const {
    os << "From: "; from_.print(os); os << " To: "; to_.print(os);
  }

 private:
  TileData from_;
  TileData to_;
};

//---

struct LineValid {
  bool    partial { false };
  QString errMsg;
};

//---

struct TileLine {
  Direction direction { Direction::NONE };
  int       start     { -1 };
  int       end       { -1 };
  int       pos       { -1 };
  int       sum       { 0 };
  int       current   { 0 };

  TileLine() = default;

  TileLine(Direction direction, int start, int end, int pos);

  int len() const;

  bool hasPosition(const TilePosition &p) const;

  TilePosition startPos() const;

  bool isValid(LineValid &lineValid) const;

  void print(std::ostream &os) const;

  void print(std::ostream &os, const QString &errMsg) const;
};

//---

struct BoardDetails {
  bool          valid   { false };
  bool          partial { false };
  int           score   { 0 };
  int           nt      { 0 };
  int           npt     { 0 };
  TilePositions validPositions;
  QString       errMsg;

  void reset();

  void addValidPosition(const TilePosition &pos);
};

struct BoardMoves {
  using Moves = std::vector<Move>;

  int   depth   { 0 };
  Moves moves;
  int   score   { 0 };
  bool  partial { false };
};

struct BestMove {
  using Moves = std::vector<Move>;

  Moves moves;
  int   score { 0 };

  bool isValid() const { return ! moves.empty(); }

  void reset() { moves.clear(); score = 0; }
};

//---

struct MoveTree {
  using Children  = std::vector<MoveTree *>;
  using MoveTrees = std::vector<const MoveTree *>;
  using ScoreTree = std::map<int,MoveTrees>;
  using Moves     = std::vector<Move>;

  MoveTree*         parent  { nullptr };
  Move              move;
  Children          children;
  bool              partial { false };
  int               score   { 0 };
  mutable ScoreTree scoreTree;

  MoveTree();

 ~MoveTree();

  void addChild(MoveTree *child) {
    child->parent = this;

    children.push_back(child);
  }

  MoveTree *root() { if (! parent) return this; return parent->root(); }

  const MoveTree *maxLeaf() const;

  void updateScoreTree(ScoreTree &scoreTree) const;

  int depth() const;

  int size() const;

  void hierMoves(Moves &move) const;

  void printDepth(std::ostream &os, int depth) const;

  void print(std::ostream &os) const;
};

//---

struct BoardLines {
  using TileLines = std::vector<TileLine>;
  using Inds      = std::set<int>;

  Inds      xinds;
  Inds      yinds;
  TileLines hlines;
  TileLines vlines;

  int score() const;
};

//---

class Board : public QWidget {
  Q_OBJECT

 public:
  using TileLines = std::vector<TileLine>;
  using Moves     = std::vector<Move>;
  using MovesList = std::vector<Moves>;

 public:
  Board(App *quinto);

  bool validPos(const TilePosition &pos) const {
    return (pos.ix >= 0 && pos.ix < quinto_->nx() && pos.iy >= 0 && pos.iy < quinto_->ny());
  }

  Tile *cellTile(const TilePosition &pos) const {
    return tiles_[pos.iy][pos.ix];
  }

  Tile *takeCellTile(const TilePosition &pos);

  void setCellTile(const TilePosition &pos, Tile *tile);

  //---

  void playBestMove(bool next=true);
  void showBestMove() const;

  void invalidateBestMove() { bestMoveValid_ = false; }

  const BestMove &getBestMove() const;

  MoveTree *boardMoveTree() const;

  bool boardMoves(BoardMoves &moves) const;

  //---

  void invalidateDetails() { detailsValid_ = false; }

  const BoardDetails &boardDetails() const;

  void getBoardLines(BoardLines &lines) const;

  //---

  void drawBoard(QPainter *painter);

  double boardTileSize () const { return bs_; }
  double playerTileSize() const { return ps_; }

 private:
  void calcBestMove();

  void calcBoardDetails();

  bool buildMoveTree(MoveTree *tree, int depth) const;

  int countTiles(const TilePosition &pos, Side side) const;

  TileData posToTileData(const QPoint &pos) const;

  void paintEvent(QPaintEvent *) override;

  void drawPlayerTiles(QPainter *painter, const PlayerP &player, int x, Qt::Alignment align);

  void drawTurn(QPainter *painter);

  void drawScores(QPainter *painter);

  void drawTile(QPainter *painter, Tile *tile, const QRectF &rect,
                double s, const QColor &bgColor, const QColor &fgColor);

  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

 private:
  using ColTiles    = std::vector<Tile *>;
  using RowColTiles = std::vector<ColTiles>;

  App*         quinto_ { nullptr };      // parent app
  QPointF      pos_    { 0.0, 0.0 };     // draw pos
  double       bs_     { 1.0 };          // board cell size
  double       ps_     { 1.0 };          // piece cell size
  RowColTiles  tiles_;                   // tile grid
  TileData     pressData_;               // mouse press data
  TileData     releaseData_;             // mouse release data
  QPoint       dragPos_;                 // drag position
  Tile*        dragTile_ { nullptr };    // drag tile
  BoardDetails details_;                 // board details
  bool         detailsValid_ { false };  // are board details current
  BestMove     bestMove_;                // best move
  bool         bestMoveValid_ { false }; // is best move current
};

//---

class Tile : public QWidget {
  Q_OBJECT

 public:
  Tile(App *quinto, int ind, int value);

  int value() const { return value_; }

  const TileOwner &owner() const { return owner_; }
  void setOwner(const TileOwner &o) { owner_ = o; }

  const TileOwner &player() const { return player_; }
  void setPlayer(const TileOwner &o) { player_ = o; }

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
  void paintEvent(QPaintEvent *) override;

 private:
  App*      quinto_ { nullptr };
  int       ind_    { -1 };
  int       value_  { -1 };
  TileOwner owner_  { TileOwner::NONE };
  TileOwner player_ { TileOwner::NONE };
  int       turn_   { -1 };
  double    s_      { 1 };
  double    fs_     { 1 };
  QFont     font_;
};

//----

class Turn {
 public:
  using Moves = std::vector<Move>;

 public:
  Turn(App *quinto, int ind);

  int ind() const { return ind_; }

  const Moves &moves() const { return moves_; }

  const Move &move(int i) { return moves_[i]; }

  void addMove(const Move &move) { moves_.push_back(move); }

  void popMove() { moves_.pop_back(); }

  void clear() { moves_.clear(); }

  int score() const;

 private:
  App*  quinto_ { nullptr };
  int   ind_    { -1 };
  Moves moves_;
};

}

#endif
