#include <CQQuinto.h>

#ifdef USE_CQAPP
#include <CQApp.h>
#else
#include <QApplication>
#endif

#include <CQPixmapCache.h>

#include <QToolButton>
#include <QPushButton>
#include <QMouseEvent>
#include <QPainter>

#include <functional>
#include <set>
#include <iostream>

#include <svg/left_svg.h>
#include <svg/close_svg.h>
#include <svg/add_svg.h>

int
main(int argc, char **argv)
{
#ifdef USE_CQAPP
  CQApp app(argc, argv);
#else
  QApplication app(argc, argv);
#endif

  bool seedRand = true;

  for (int i = 1; i < argc; ++i) {
    QString arg = argv[i];

    if (arg == "-noseed")
      seedRand = false;
  }

  if (seedRand)
    srand(time(nullptr));

  CQQuinto quinto;

  quinto.resize(quinto.sizeHint());

  quinto.show();

  return app.exec();
}

//---

CQQuinto::
CQQuinto(QWidget *parent) :
 QFrame(parent)
{
  setObjectName("quinto");

  tileSet_ = new CQQuintoTileSet(this);

  player1_ = new CQQuintoPlayer(this, 0, CQQuintoPlayer::Type::HUMAN);
  player2_ = new CQQuintoPlayer(this, 1, CQQuintoPlayer::Type::COMPUTER);

  player1_->setName("Player"  );
  player2_->setName("Computer");

  turn_ = new CQQuintoTurn(this, 0);

  currentPlayer_ = player1_;

  //---

  board_ = new CQQuintoBoard(this);

  //---

  auto createToolButton = [&](const QString &name, const QString &icon,
                             const char *slot) -> QToolButton *{
    QToolButton *button = new QToolButton(this);

    button->setObjectName (name);
    button->setFocusPolicy(Qt::NoFocus);

    button->setIcon(CQPixmapCacheInst->getIcon(icon));

    connect(button, SIGNAL(clicked()), this, slot);

    return button;
  };

  cancelButton_ = createToolButton("cancel", "CLOSE", SLOT(cancelSlot()));
  backButton_   = createToolButton("back"  , "LEFT" , SLOT(backSlot()));
  applyButton_  = createToolButton("apply" , "ADD"  , SLOT(applySlot()));

  //---

  auto createPushButton = [&](const QString &name, const QString &text,
                              const char *slot) -> QPushButton *{
    QPushButton *button = new QPushButton(this);

    button->setObjectName (name);
    button->setFocusPolicy(Qt::NoFocus);

    button->setText(text);

    connect(button, SIGNAL(clicked()), this, slot);

    return button;
  };

  newGameButton_ = createPushButton("newGame", "New Game", SLOT(newGameSlot()));

  //---

  player1_->drawTiles();
  player2_->drawTiles();

  //---

  updateState();
}

CQQuinto::
~CQQuinto()
{
  delete tileSet_;

  delete player1_;
  delete player2_;

  delete board_;

  for (auto &turn : turns_)
    delete turn;

  delete turn_;
}

CQQuintoTile *
CQQuinto::
getTile()
{
  return tileSet_->getTile();
}

void
CQQuinto::
addMove(const CQQuintoMove &move)
{
  turn_->addMove(move);
}

void
CQQuinto::
resizeEvent(QResizeEvent *)
{
  updateWidgets();
}

void
CQQuinto::
updateState()
{
  updateWidgets();

  update();

  //---

  //showBestMove();
}

void
CQQuinto::
updateFonts()
{
  auto invalidateToolButtonSizeHint = [](QToolButton *w) {
    w->setToolButtonStyle(Qt::ToolButtonIconOnly);
    w->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  };

  auto invalidatePushButtonSizeHint = [](QPushButton *w) {
    w->setFlat(true);
    w->setFlat(false);
  };

  double fs = calcFontScale(board_->playerTileSize());

  if (fs == lastFs_)
    return;

  lastFs_ = fs;

  QFont font;

  double ps = font.pointSizeF()*fs;

  font.setPointSizeF(ps);

  QSize is(ps, ps);

  cancelButton_->setIconSize(is);
  //cancelButton_->setIcon(CQPixmapCacheInst->getIcon("CLOSE"));

  backButton_->setIconSize(is);
  //backButton_->setIcon(CQPixmapCacheInst->getIcon("LEFT"));

  applyButton_->setIconSize(is);
  //applyButton_->setIcon(CQPixmapCacheInst->getIcon("ADD"));

  invalidateToolButtonSizeHint(cancelButton_);
  invalidateToolButtonSizeHint(backButton_  );
  invalidateToolButtonSizeHint(applyButton_ );

  QSize sh = cancelButton_->sizeHint();

  cancelButton_->resize(sh);
  backButton_  ->resize(sh);
  applyButton_ ->resize(sh);

  int bw = sh.width();

  if (isShowComputer()) {
    cancelButton_->move(width()/2 - 3*bw/2 - 2, 2);
    backButton_  ->move(width()/2 -   bw/2    , 2);
    applyButton_ ->move(width()/2 +   bw/2 + 2, 2);
  }
  else {
    cancelButton_->move(width() - 1 - 3*bw - 2, 2);
    backButton_  ->move(width() - 1 - 2*bw - 2, 2);
    applyButton_ ->move(width() - 1 -   bw - 2, 2);
  }

  //---

  newGameButton_->setFont(font);

  invalidatePushButtonSizeHint(newGameButton_);

  QSize nsh = newGameButton_->sizeHint();

  newGameButton_->resize(nsh);

  newGameButton_->move(width()/2 - nsh.width()/2, height() - 1 - nsh.height() - 4);
}

void
CQQuinto::
showBestMove() const
{
  board_->showBestMove();
}

void
CQQuinto::
updateWidgets()
{
  board_->move(0, 0);

  board_->resize(width(), height());

  board_->show();

  int bw = backButton_->sizeHint().width();

  if (isShowComputer()) {
    cancelButton_->move(width()/2 - 3*bw/2 - 2, 2);
    backButton_  ->move(width()/2 -   bw/2    , 2);
    applyButton_ ->move(width()/2 +   bw/2 + 2, 2);
  }
  else {
    cancelButton_->move(width() - 1 - 3*bw - 2, 2);
    backButton_  ->move(width() - 1 - 2*bw - 2, 2);
    applyButton_ ->move(width() - 1 -   bw - 2, 2);
  }

  cancelButton_->show(); cancelButton_->raise();
  backButton_  ->show(); backButton_  ->raise();
  applyButton_ ->show(); applyButton_ ->raise();

  //---

  QSize nsh = newGameButton_->sizeHint();

  newGameButton_->move(width()/2 - nsh.width()/2, height() - 1 - nsh.height() - 4);

  newGameButton_->show(); newGameButton_->raise();

  //---

  int score;

  bool valid   = isTurnValid(score);
  bool canUndo = ! turn_->moves().empty();

  cancelButton_->setEnabled(canUndo);
  backButton_  ->setEnabled(canUndo);
  applyButton_ ->setEnabled(valid);
}

void
CQQuinto::
cancelSlot()
{
  int n = turn_->moves().size();

  for (int i = n - 1; i >= 0; --i) {
    const CQQuintoMove &move = turn_->move(i);

    undoMove(move);
  }

  turn_->clear();

  //---

  updateState();
}

void
CQQuinto::
backSlot()
{
  int n = turn_->moves().size();

  if (n > 0) {
    const CQQuintoMove &move = turn_->move(n - 1);

    undoMove(move);

    turn_->popMove();
  }

  //---

  updateState();
}

void
CQQuinto::
applySlot()
{
  assert(currentPlayer_->type() == CQQuintoPlayer::Type::HUMAN);

  apply();
}

void
CQQuinto::
apply()
{
  // check valid
  int score;

  if (! isTurnValid(score))
    return;

  // update score
  currentPlayer_->addScore(score);

  // get new tiles
  currentPlayer_->drawTiles();

  //---

  nextTurn();

  //---

  updateState();

  //---

  if (currentPlayer_->type() == CQQuintoPlayer::Type::COMPUTER) {
    while (canMove()) {
      // auto play computers best move
      computerMove();

      // if player can move computer is done
      if (canMove())
        break;

      nextTurn();
    }

    if (currentPlayer_->type() == CQQuintoPlayer::Type::COMPUTER) {
      nextTurn();

      // if player can't move then game over
      if (! canMove()) {
        gameOver();
      }
    }
  }
}

void
CQQuinto::
newGameSlot()
{
  newGame();
}

void
CQQuinto::
newGame()
{
  const int handSize = this->handSize();

  // move all board and player tiles back to tile set
  for (int i = 0; i < handSize; ++i) {
    CQQuintoTile *tile1 = player1_->takeTile(i, /*nocheck*/true);
    CQQuintoTile *tile2 = player2_->takeTile(i, /*nocheck*/true);

    if (tile1) tileSet_->ungetTile(tile1);
    if (tile2) tileSet_->ungetTile(tile2);
  }

  const int nx = this->nx();
  const int ny = this->ny();

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      CQQuintoTile *tile = board_->takeCellTile(ix, iy);

      if (tile) tileSet_->ungetTile(tile);
    }
  }

  //---

  // reset current player and player scores
  currentPlayer_ = player1_;

  player1_->setScore(0);
  player2_->setScore(0);

  //---

  // reset turns
  for (auto &turn : turns_)
    delete turn;

  turns_.clear();

  delete turn_;

  turn_ = new CQQuintoTurn(this, 0);

  //---

  // shuffle tiles
  tileSet_->shuffle();

  //---

  // redraw player tiles
  player1_->drawTiles();
  player2_->drawTiles();

  //---

  gameOver_ = false;

  newGameButton_->setText("New Game");

  //---

  updateState();
}

void
CQQuinto::
computerMove()
{
  assert(currentPlayer_->type() == CQQuintoPlayer::Type::COMPUTER);

  board_->playBestMove();
}

void
CQQuinto::
nextTurn()
{
  // next turn
  turns_.push_back(turn_);

  int ind = turn_->ind() + 1;

  turn_ = new CQQuintoTurn(this, ind);

  //---

  // switch to next player
  currentPlayer_ = (currentPlayer_ == player1_ ? player2_ : player1_);
}

void
CQQuinto::
gameOver()
{
  gameOver_ = true;

  newGameButton_->setText("Game Over");

  updateState();

}

bool
CQQuinto::
canMove() const
{
  CQQuintoBoard::BoardDetails details;

  board_->boardDetails(details);

  if (! details.valid)
    return false;

  if (details.validPositions.empty())
    return false;

  if (currentPlayer_->numTiles() == 0)
    return false;

  return true;
}

bool
CQQuinto::
isTurnValid(int &score) const
{
  CQQuintoBoard::BoardDetails details;

  board_->boardDetails(details);

  score = details.score;

  return details.valid && ! details.partial;
}

void
CQQuinto::
undoMove(const CQQuintoMove &move)
{
  const CQQuinto::TileData &from = move.from();
  const CQQuinto::TileData &to   = move.to  ();

  doMove(to, from);
}

void
CQQuinto::
doMove(const CQQuintoMove &move)
{
  const CQQuinto::TileData &from = move.from();
  const CQQuinto::TileData &to   = move.to  ();

  doMove(from, to);
}

void
CQQuinto::
doMove(const CQQuinto::TileData &from, const CQQuinto::TileData &to)
{
  if      (from.owner == CQQuinto::TileOwner::PLAYER1) {
    assert(to.owner == TileOwner::BOARD);

    CQQuintoTile *fromTile = player1_->tile(from.x);
    assert(fromTile);

    CQQuintoCell *toCell = board_->cell(to.x, to.y);
    assert(! toCell->tile());

    fromTile = player1_->takeTile(from.x);

    fromTile->setOwner (CQQuintoTile::Owner::BOARD);
    fromTile->setPlayer(from.owner);
    fromTile->setTurn  (turn()->ind());

    toCell->setTile(fromTile);
  }
  else if (from.owner == CQQuinto::TileOwner::PLAYER2) {
    assert(to.owner == TileOwner::BOARD);

    CQQuintoTile *fromTile = player2_->tile(from.x);
    assert(fromTile);

    CQQuintoCell *toCell = board_->cell(to.x, to.y);
    assert(! toCell->tile());

    fromTile = player2_->takeTile(from.x);

    fromTile->setOwner (CQQuintoTile::Owner::BOARD);
    fromTile->setPlayer(from.owner);
    fromTile->setTurn  (turn()->ind());

    toCell->setTile(fromTile);
  }
  else if (from.owner == CQQuinto::TileOwner::BOARD) {
    if      (to.owner == CQQuinto::TileOwner::PLAYER1 ||
             to.owner == CQQuinto::TileOwner::PLAYER2) {
      CQQuintoCell *fromCell = board_->cell(from.x, from.y);

      CQQuintoTile *fromTile = fromCell->tile();
      assert(fromTile);

      fromCell->setTile(nullptr);

      if (to.owner == CQQuinto::TileOwner::PLAYER1)
        player1_->addTile(fromTile, to.x);
      else
        player2_->addTile(fromTile, to.x);

      fromTile->setOwner (to.owner);
      fromTile->setPlayer(CQQuinto::TileOwner::NONE);
    }
    else if (to.owner == CQQuinto::TileOwner::BOARD) {
      CQQuintoCell *fromCell = board_->cell(from.x, from.y);

      CQQuintoTile *fromTile = fromCell->tile();
      assert(fromTile);

      CQQuintoCell *toCell = board_->cell(to.x, to.y);

      CQQuintoTile *toTile = toCell->tile();
      assert(! toTile);

      fromCell->setTile(nullptr);
      toCell  ->setTile(fromTile);
    }
  }
  else {
    assert(false);
  }
}

int
CQQuinto::
moveScore(const CQQuintoMove &move) const
{
  const CQQuinto::TileData &to = move.to  ();

  if (to.owner == CQQuinto::TileOwner::BOARD) {
    CQQuintoCell *cell = board_->cell(to.x, to.y);

    CQQuintoTile *tile = cell->tile();

    if (tile)
      return tile->value();
  }

  return 0;
}

double
CQQuinto::
calcFontScale(double s) const
{
  QFont font;

  QFontMetricsF fm(font);

  double w = fm.width("8");
  double h = fm.height();

  return s/std::max(w, h);
}

QSize
CQQuinto::
sizeHint() const
{
  return QSize(1200, 1200);
}

//---

template<typename FN, typename... ARGS>
auto curry(FN fn, ARGS... args) {
  return [=](auto... rest) { return fn(args..., rest...); };
}

template<typename FN>
auto repeatFn(int n, FN fn) {
  for (int i = 0; i < n; ++i)
    fn();
}

CQQuintoTileSet::
CQQuintoTileSet(CQQuinto *quinto) :
 quinto_(quinto)
{
  auto addTile = [&](int value) {
    int ind = tiles_.size();

    CQQuintoTile *tile = new CQQuintoTile(quinto_, ind, value);

    tile->setOwner (CQQuinto::TileOwner::TILE_SET);
    tile->setPlayer(CQQuinto::TileOwner::NONE);

    tiles_.push_back(tile);

    itiles_.push_back(tile);
  };

  /* 7  - #0 Tiles */ repeatFn( 7, curry(addTile, 0));
  /* 6  - #1 Tiles */ repeatFn( 6, curry(addTile, 1));
  /* 6  - #2 Tiles */ repeatFn( 6, curry(addTile, 2));
  /* 7  - #3 Tiles */ repeatFn( 7, curry(addTile, 3));
  /* 10 - #4 Tiles */ repeatFn(10, curry(addTile, 4));
  /* 6  - #5 Tiles */ repeatFn( 6, curry(addTile, 5));
  /* 10 - #6 Tiles */ repeatFn(10, curry(addTile, 6));
  /* 14 - #7 Tiles */ repeatFn(14, curry(addTile, 7));
  /* 12 - #8 Tiles */ repeatFn(12, curry(addTile, 8));
  /* 12 - #9 Tiles */ repeatFn(12, curry(addTile, 9));

  shuffle();
}

CQQuintoTileSet::
~CQQuintoTileSet()
{
  for (auto &tile : itiles_)
    delete tile;
}

void
CQQuintoTileSet::
shuffle()
{
  int nt = tiles_.size();

  int ns = 100;

  for (int i = 0; i < ns; ++i) {
    int pos1 = rand() % nt;

    int pos2 = rand() % nt;

    while (pos1 == pos2)
      pos2 = rand() % nt;

    std::swap(tiles_[pos1], tiles_[pos2]);
  }
}

CQQuintoTile *
CQQuintoTileSet::
getTile()
{
  if (tiles_.empty())
    return nullptr;

  CQQuintoTile *tile = tiles_.back();

  tile->setOwner (CQQuintoTile::Owner::NONE);
  tile->setPlayer(CQQuintoTile::Owner::NONE);

  tiles_.pop_back();

  return tile;
}

void
CQQuintoTileSet::
ungetTile(CQQuintoTile *tile)
{
  tile->setOwner (CQQuintoTile::Owner::TILE_SET);
  tile->setPlayer(CQQuintoTile::Owner::NONE);

  tiles_.push_back(tile);
}

//---

CQQuintoTile::
CQQuintoTile(CQQuinto *quinto, int ind, int value) :
 quinto_(quinto), ind_(ind), value_(value)
{
  setObjectName(QString("tile.%1").arg(ind_));

  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);

  hide();
}

void
CQQuintoTile::
show(const QRect &rect)
{
  resize(rect.size());

  move(rect.topLeft());

  QWidget::show();
}

void
CQQuintoTile::
hide()
{
  setVisible(false);
}

void
CQQuintoTile::
setSize(double s)
{
  if (s_ != s) {
    s_ = s;

    QFont font;

    setFontScale(quinto_->calcFontScale(s_));

    font.setPointSizeF(font.pointSizeF()*fontScale());

    font_ = font;
  }
}

void
CQQuintoTile::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);

  drawTile(&painter);
}

void
CQQuintoTile::
drawTile(QPainter *painter)
{
  painter->setPen(Qt::black);

  drawTile(painter, rect());
}

void
CQQuintoTile::
drawTile(QPainter *painter, const QRectF &rect)
{
  painter->drawRect(rect);

  painter->setFont(font_);

  QFontMetricsF fm(painter->font());

  int value = this->value();

  QString text = QString("%1").arg(value);

  double tx = rect.center().x() - fm.width(text)/2.0;
  double ty = rect.center().y() + (fm.ascent() - fm.descent())/2;

  painter->drawText(QPointF(tx, ty), text);
}

//---

CQQuintoPlayer::
CQQuintoPlayer(CQQuinto *quinto, int ind, Type type) :
 quinto_(quinto), ind_(ind), type_(type)
{
  const int handSize = quinto_->handSize();

  for (int i = 0; i < handSize; ++i)
    tiles_.push_back(nullptr);

  if (ind == 0)
    name_ = "P1";
  else
    name_ = "P2";
}

int
CQQuintoPlayer::
numTiles() const
{
  const int handSize = quinto_->handSize();

  int n = 0;

  for (int i = 0; i < handSize; ++i)
    if (tiles_[i])
      ++n;

  return n;
}

void
CQQuintoPlayer::
drawTiles()
{
  int nt = tiles_.size();

  for (int i = 0; i < nt; ++i) {
    if (tiles_[i])
      continue;

    CQQuintoTile *tile = quinto_->getTile();

    if (! tile)
      break;

    tile->setOwner (owner());
    tile->setPlayer(CQQuinto::TileOwner::NONE);

    tiles_[i] = tile;
  }
}

CQQuintoTile *
CQQuintoPlayer::
tile(int i) const
{
  const int handSize = quinto_->handSize();

  assert(i >= 0 && i < handSize);

  return tiles_[i];
}

CQQuintoTile *
CQQuintoPlayer::
takeTile(int i, bool nocheck)
{
  const int handSize = quinto_->handSize();

  assert(i >= 0 && i < handSize);

  CQQuintoTile *tile = tiles_[i];

  if (nocheck) {
    if (! tile)
      return nullptr;
  }
  else {
    assert(tile);
  }

  tile->setOwner (CQQuintoTile::Owner::NONE);
  tile->setPlayer(CQQuinto::TileOwner::NONE);

  tiles_[i] = nullptr;

  return tile;
}

void
CQQuintoPlayer::
addTile(CQQuintoTile *tile, int i)
{
  const int handSize = quinto_->handSize();

  assert(i >= 0 && i < handSize);

  assert(! tiles_[i]);

  tiles_[i] = tile;

  tile->setOwner (owner());
  tile->setPlayer(CQQuinto::TileOwner::NONE);
}

CQQuinto::TileOwner
CQQuintoPlayer::
owner() const
{
  return (this == quinto_->player1() ? CQQuinto::TileOwner::PLAYER1 : CQQuinto::TileOwner::PLAYER2);
}

//---

CQQuintoBoard::
CQQuintoBoard(CQQuinto *quinto) :
 QWidget(quinto), quinto_(quinto)
{
  setObjectName("board");

  setFocusPolicy(Qt::StrongFocus);

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  cells_.resize(ny);

  for (int iy = 0; iy < ny; ++iy) {
    cells_[iy].resize(nx);

    for (int ix = 0; ix < nx; ++ix)
      cells_[iy][ix] = new CQQuintoCell(ix, iy);
  }
}

CQQuintoBoard::
~CQQuintoBoard()
{
  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  for (int iy = 0; iy < ny; ++iy)
    for (int ix = 0; ix < nx; ++ix)
      delete cells_[iy][ix];
}

CQQuintoTile *
CQQuintoBoard::
cellTile(int x, int y) const
{
  if (x >= 0 && x < quinto_->nx() && y >= 0 && y < quinto_->ny())
    return cells_[y][x]->tile();

  return nullptr;
}

CQQuintoTile *
CQQuintoBoard::
takeCellTile(int x, int y)
{
  if (x >= 0 && x < quinto_->nx() && y >= 0 && y < quinto_->ny()) {
    CQQuintoTile *tile = cells_[y][x]->tile();

    if (! tile)
      return nullptr;

    cells_[y][x]->setTile(nullptr);

    tile->setOwner (CQQuinto::TileOwner::NONE);
    tile->setPlayer(CQQuinto::TileOwner::NONE);

    return tile;
  }

  return nullptr;
}

void
CQQuintoBoard::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);

  drawBoard(&painter);

  quinto_->updateFonts();
}

void
CQQuintoBoard::
drawBoard(QPainter *painter)
{
  const int b = 4;

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  //---

  double w = width () - 2*b;
  double h = height() - 3*b;

  int ny1 = ny + 1; // for player tiles

  bs_ = std::min(w/nx, h/ny1);

  x_ = (w - nx *boardTileSize())/2 + b;
  y_ = (h - ny1*boardTileSize())/2 + 2*b + boardTileSize();

  ps_ = boardTileSize()*0.6;

  //----

  // draw turn
  if (! quinto_->isShowComputer())
    drawTurn(painter);

  //----

  // draw player tiles
  CQQuintoPlayer *player1 = quinto_->player1();
  CQQuintoPlayer *player2 = quinto_->player2();

  if (quinto_->isShowComputer()) {
    drawPlayerTiles(painter, player1, Qt::AlignLeft );
    drawPlayerTiles(painter, player2, Qt::AlignRight);
  }
  else {
    CQQuintoPlayer *player =
      (player1->type() == CQQuintoPlayer::Type::HUMAN ? player1 : player2);

    drawPlayerTiles(painter, player, Qt::AlignHCenter);
  }

  //----

  boardDetails(details_);

  //----

  // draw board tiles
  int turnInd = quinto_->turn()->ind();

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      TilePosition pos(ix, iy);

      QRectF rect(x_ + ix*boardTileSize(), y_ + iy*boardTileSize(),
                  boardTileSize(), boardTileSize());

      CQQuintoCell *cell = cells_[iy][ix];

      CQQuintoTile *tile = cell->tile();

      bool current = (tile && tile->turn() == turnInd);
      bool valid   = (details_.validPositions.find(pos) != details_.validPositions.end());

      QBrush boardTileBrush(Qt::NoBrush);

      if (tile) {
        if (current) {
          if (! details_.valid)
            boardTileBrush = quinto_->invalidTileColor();
          else
            boardTileBrush = quinto_->currentTileColor();
        }
        else {
          if (tile->player() == CQQuinto::TileOwner::PLAYER1)
            boardTileBrush = quinto_->player1TileColor();
          else
            boardTileBrush = quinto_->player2TileColor();
        }
      }
      else if (valid) {
        boardTileBrush = quinto_->validMoveColor();
      }

      drawTile(painter, tile, rect, boardTileSize(), boardTileBrush);
    }
  }

  //---

  drawScores(painter);
}

void
CQQuintoBoard::
drawTurn(QPainter *painter)
{
  const int b = 4;

  // set font
  QFont font;

  double fs = quinto_->calcFontScale(playerTileSize());

  font.setPointSizeF(font.pointSizeF()*fs);

  painter->setFont(font);

  QFontMetricsF fm(painter->font());

  QString turnText = QString("Turn: %1").arg(quinto_->turn()->ind() + 1);

  int dbt = playerTileSize() - fm.height();

  double x = b;
  double y = b + dbt/2 + fm.ascent();

  painter->drawText(x, y, turnText);
}

void
CQQuintoBoard::
drawScores(QPainter *painter)
{
  const int b = 4;

  CQQuintoPlayer *player1 = quinto_->player1();
  CQQuintoPlayer *player2 = quinto_->player2();

  // set font
  QFont font;

  double fs = quinto_->calcFontScale(playerTileSize());

  font.setPointSizeF(font.pointSizeF()*fs);

  painter->setFont(font);

  QFontMetricsF fm(painter->font());

  //---

  // draw scores
  int p1score = player1->score();
  int p1extra = 0;

  int p2score = player2->score();
  int p2extra = 0;

  QString score1Text = QString("%1").arg(p1score);
  QString score2Text = QString("%1").arg(p2score);

  if (details_.valid) {
    CQQuintoPlayer *currentPlayer = quinto_->currentPlayer();

    if (currentPlayer == player1) {
      p1extra = details_.score;

      score1Text += QString(" (%1)").arg(p1extra);
    }
    else {
      p2extra = details_.score;

      score2Text += QString(" (%1)").arg(p2extra);
    }
  }

  QString p1Title = QString("%1: ").arg(player1->name());
  QString p2Title = QString("%1: ").arg(player2->name());

  int sy = height() - 1 - fm.descent() - 2;

  // draw player 1 score
  int sx1 = 2*b;

  painter->drawText(sx1, sy, p1Title);

  sx1 += fm.width(p1Title) + 2*b;

  painter->drawText(sx1, sy, score1Text);

  // draw player 2 score
  int sx2 = width() - 2*b - fm.width(score2Text);

  painter->drawText(sx2, sy, score2Text);

  sx2 -= fm.width(p2Title) + 2*b;

  painter->drawText(sx2, sy, p2Title);
}

void
CQQuintoBoard::
drawPlayerTiles(QPainter *painter, CQQuintoPlayer *player, Qt::Alignment align)
{
  const int b = 4;

  const int handSize = quinto_->handSize();

  //---

  // calc font size
  QFont font;

  double fs = quinto_->calcFontScale(playerTileSize());

  font.setPointSizeF(font.pointSizeF()*fs);

  painter->setFont(font);

  QFontMetricsF fm(painter->font());

  //---

  // calc name and tile position
  QString playerText = player->name();

  int tx, px;

  if      (align == Qt::AlignLeft) {
    tx = b;
    px = tx + fm.width(playerText) + 2*b;
  }
  else if (align == Qt::AlignRight) {
    tx = width() - 1 - b - fm.width(playerText);
    px = tx - 2*b - handSize*playerTileSize();
  }
  else {
    tx = width()/2 - fm.width(playerText)/2 - handSize*playerTileSize()/2;
    px = tx + fm.width(playerText) + 2*b;
  }

  int ty = b;

  player->setTileX(px);
  player->setTileY(ty);

  //---

  // draw player name
  if (quinto_->isShowComputer()) {
    CQQuintoPlayer *currentPlayer = quinto_->currentPlayer();

    painter->setPen(player == currentPlayer ? quinto_->currentPlayerColor() : Qt::black);
  }
  else
    painter->setPen(Qt::black);

  painter->setFont(font);

  int dbt = playerTileSize() - fm.height();

  painter->drawText(tx, player->tileY() + dbt/2 + fm.ascent(), playerText);

  //---

  // draw player tiles
  QBrush playerTileBrush;

  if (player->owner() == CQQuinto::TileOwner::PLAYER1)
    playerTileBrush = quinto_->player1TileColor();
  else
    playerTileBrush = quinto_->player2TileColor();

  for (int i = 0; i < handSize; ++i) {
    double x = px + i*playerTileSize();

    QRectF rect(x, player->tileY(), playerTileSize(), playerTileSize());

    CQQuintoTile *tile = player->tile(i);

    drawTile(painter, tile, rect, playerTileSize(), playerTileBrush);
  }
}

void
CQQuintoBoard::
drawTile(QPainter *painter, CQQuintoTile *tile, const QRectF &rect, double s, const QBrush &brush)
{
  painter->setBrush(brush);
  painter->setPen  (Qt::black);

  if (tile) {
    tile->setSize(s);

    tile->drawTile(painter, rect);
  }
  else
    painter->drawRect(rect);
}

void
CQQuintoBoard::
playBestMove()
{
  Moves moves;
  int   score;

  if (! getBestMove(moves, score))
    return;

  int nm = moves.size();

  for (int i = 0; i < nm; ++i) {
    quinto_->doMove(moves[i]);
  }

  quinto_->updateState();

  quinto_->apply();
}

void
CQQuintoBoard::
showBestMove() const
{
  Moves moves;
  int   score;

  if (! getBestMove(moves, score))
    return;

  std::cerr << "Best Moves:";

  int nm = moves.size();

  for (int i = 0; i < nm; ++i) {
    std::cerr << " ";

    moves[i].print(std::cerr);
  }

  std::cerr << " @" << score << "\n";
}

bool
CQQuintoBoard::
getBestMove(Moves &moves, int &score) const
{
  CQQuintoBoard::MoveTree *moveTree = boardMoveTree();

  if (! moveTree)
    return false;

  //std::cerr << "Move Tree: "; moveTree->print(std::cerr); std::cerr << "\n";

  const CQQuintoBoard::MoveTree *maxLeaf = moveTree->maxLeaf();

  if (maxLeaf) {
    moves.push_back(maxLeaf->move);

    MoveTree *parent = maxLeaf->parent;

    while (parent) {
      if (parent->move.isValid())
        moves.push_back(parent->move);

      parent = parent->parent;
    }

    int nm = moves.size();

    for (int i = 0; i < nm/2; ++i)
      std::swap(moves[i], moves[nm - i - 1]);

    score = maxLeaf->score;
  }

  //---

  delete moveTree;

  return true;
}

CQQuintoBoard::MoveTree *
CQQuintoBoard::
boardMoveTree() const
{
  return buildBoardMoveTree(0);
}

CQQuintoBoard::MoveTree *
CQQuintoBoard::
buildBoardMoveTree(int depth) const
{
  Moves moves;
  int   score;
  bool  partial;

  if (! boardMoves(moves, score, partial))
    return nullptr;

  MoveTree *tree = new MoveTree;

  tree->partial = partial;
  tree->score   = score;

  for (auto &move : moves) {
    quinto_->doMove(move.from(), move.to());

    MoveTree *childTree = buildBoardMoveTree(depth + 1);

    if (childTree) {
      childTree->parent = tree;
      childTree->move   = move;

      tree->children.push_back(childTree);
    }

    quinto_->doMove(move.to(), move.from());
  }

  return tree;
}

bool
CQQuintoBoard::
boardMoves(Moves &moves, int &score, bool &partial) const
{
  BoardDetails details;

  boardDetails(details);

  if (! details.valid)
    return false;

  //---

  score   = details.score;
  partial = details.partial;

  //---

  const int handSize = quinto_->handSize();

  CQQuintoPlayer *currentPlayer = quinto_->currentPlayer();

  CQQuinto::TileOwner playerOwner = currentPlayer->owner();

  for (const auto &position : details.validPositions) {
    for (int i = 0; i < handSize; ++i) {
      if (currentPlayer->tile(i)) {
        CQQuinto::TileData from(playerOwner, i, 0);
        CQQuinto::TileData to  (CQQuinto::TileOwner::BOARD, position);

        CQQuintoMove move(from, to);

        moves.push_back(move);
      }
    }
  }

  return true;
}

void
CQQuintoBoard::
boardDetails(BoardDetails &details) const
{
  auto addValidPosition = [&](int ix, int iy) {
    assert(! cellTile(ix, iy));

    details.validPositions.insert(TilePosition(ix, iy));
  };

  //---

  details.reset();

  details.valid   = true;
  details.partial = false;

  //---

  int turnInd = quinto_->turn()->ind();

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  //---

  // count number of current move tiles in each row (y) and column (x)
  int nt = 0, npt = 0;

  using Inds = std::set<int>;

  Inds xinds, yinds;

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      CQQuintoTile *tile = cellTile(ix, iy);

      if (tile)
        ++nt;

      if (tile && tile->turn() == turnInd) {
        xinds.insert(ix);
        yinds.insert(iy);

        ++npt;
      }
    }
  }

  //---

  // if board empty then must be valid
  // return center positions for valid positions
  if (nt == 0) {
#if 0
    // multiple center positions
    int ix1 = (nx - 1)/2, ix2 = nx/2;
    int iy1 = (ny - 1)/2, iy2 = ny/2;

    addValidPosition(ix1, iy1);
    addValidPosition(ix2, iy1);
    addValidPosition(ix1, iy2);
    addValidPosition(ix2, iy2);
#else
    // single center position
    int ix1 = (nx - 1)/2;
    int iy1 = (ny - 1)/2;

    addValidPosition(ix1, iy1);
#endif

    details.partial = true;

    return;
  }

  //----

  // no tiles placed yet (for current player) then must be valid,
  if (npt == 0) {
    assert(xinds.empty() && yinds.empty());

    // play off existing pieces (board not empty)
    for (int iy = 0; iy < ny; ++iy) {
      for (int ix = 0; ix < nx; ++ix) {
        CQQuintoTile *tile = cellTile(ix, iy);
        if (tile) continue;

        // check if empty cell has any surrounding tiles
        CQQuintoTile *l_tile = cellTile(ix - 1, iy    );
        CQQuintoTile *r_tile = cellTile(ix + 1, iy    );
        CQQuintoTile *t_tile = cellTile(ix    , iy - 1);
        CQQuintoTile *b_tile = cellTile(ix    , iy + 1);

        if (! l_tile && ! r_tile && ! t_tile && ! b_tile)
          continue;

        //---

        // count run to left, right, top, bottom
        int l_count = countTiles(ix - 1, iy    , Side::LEFT  );
        int r_count = countTiles(ix + 1, iy    , Side::RIGHT );
        int t_count = countTiles(ix    , iy - 1, Side::TOP   );
        int b_count = countTiles(ix    , iy + 1, Side::BOTTOM);

        if (l_count + r_count + 1 > 5)
          continue;
        if (t_count + b_count + 1 > 5)
          continue;

        addValidPosition(ix, iy);
      }
    }

    // can't apply yet
    details.partial = true;

    return;
  }

  //---

  // get connected lines, length 2 or more, including at least one turn piece
  TileLines hlines, vlines;

  getBoardLines(hlines, vlines);

  //---

  // check all lines
  for (const auto &line : hlines) {
    details.valid = line.isValid(details.partial, details.errMsg);

    //line.print(std::cerr, details.errMsg); std::cerr << "\n";

    if (! details.valid)
      return;
  }

  for (const auto &line : vlines) {
    details.valid = line.isValid(details.partial, details.errMsg);

    //line.print(std::cerr, details.errMsg); std::cerr << "\n";

    if (! details.valid)
      return;
  }

  //---

  // single piece played then check row or column
  if (npt == 1) {
    assert(xinds.size() == 1 && yinds.size() == 1);

    int ix1 = *xinds.begin();
    int iy1 = *yinds.begin();

    //---

    // play off existing pieces (board not empty)
    for (int iy = 0; iy < ny; ++iy) {
      for (int ix = 0; ix < nx; ++ix) {
        CQQuintoTile *tile = cellTile(ix, iy);
        if (tile) continue;

        // check if empty cell has any surrounding tiles
        CQQuintoTile *l_tile = cellTile(ix - 1, iy    );
        CQQuintoTile *r_tile = cellTile(ix + 1, iy    );
        CQQuintoTile *t_tile = cellTile(ix    , iy - 1);
        CQQuintoTile *b_tile = cellTile(ix    , iy + 1);

        if (! l_tile && ! r_tile && ! t_tile && ! b_tile)
          continue;

        //---

        // count run to left, right, top, bottom
        int l_count = countTiles(ix - 1, iy    , Side::LEFT  );
        int r_count = countTiles(ix + 1, iy    , Side::RIGHT );
        int t_count = countTiles(ix    , iy - 1, Side::TOP   );
        int b_count = countTiles(ix    , iy + 1, Side::BOTTOM);

        if (l_count + r_count + 1 > 5)
          continue;
        if (t_count + b_count + 1 > 5)
          continue;

        //---

        // line must include our single piece
        int ixl1 = ix - 1 - l_count, ixr1 = ix + 1 + r_count;
        int iyt1 = iy - 1 - t_count, iyb1 = iy + 1 + b_count;

        if ((ix1 == ix && iy1 >= iyt1 && iy1 <= iyb1) ||
            (iy1 == iy && ix1 >= ixl1 && ix1 <= ixr1)) {
          addValidPosition(ix, iy);
        }
      }
    }

    return;
  }

  //---

  // two or more pieces. must be in a single row or column
  if (xinds.size() > 1 && yinds.size() > 1) {
    details.valid  = false;
    details.errMsg = "Disjoint pieces";
    return;
  }

  //---

  bool horizontal = (xinds.size() > 1);

  //---

  if (horizontal)
    details.lines = hlines;
  else
    details.lines = vlines;

  assert(! details.lines.empty());

  //---

  // score all lines
  for (const auto &line : hlines)
    details.score += line.sum;

  for (const auto &line : vlines)
    details.score += line.sum;

  //---

  // add valid positions (end of lines)
  for (const auto &line : details.lines) {
    if (line.len() >= 5)
      continue;

    if (horizontal) {
      int ix1 = line.start - 1;
      int ix2 = line.end   + 1;

      if (ix1 >= 0 ) addValidPosition(ix1, line.pos);
      if (ix2 <  nx) addValidPosition(ix2, line.pos);
    }
    else {
      int iy1 = line.start - 1;
      int iy2 = line.end   + 1;

      if (iy1 >= 0 ) addValidPosition(line.pos, iy1);
      if (iy2 <  ny) addValidPosition(line.pos, iy2);
    }
  }
}

void
CQQuintoBoard::
getBoardLines(TileLines &hlines, TileLines &vlines) const
{
  TileLine shline, svline;

  int turnInd = quinto_->turn()->ind();

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  // get horizontal sequence
  for (int iy = 0; iy < ny; ++iy) {
    int ix = 0;

    while (ix < nx) {
      // find first tile
      while (ix < nx && ! cellTile(ix, iy))
        ++ix;

      if (ix >= nx)
        break;

      int ixs = ix;

      // find last tile
      while (ix < nx && cellTile(ix, iy))
        ++ix;

      int ixe = ix - 1;

      TileLine line(Direction::HORIZONTAL, ixs, ixe, iy);

      //---

      // ignore unit line
      if (line.len() == 1) {
        shline = line;
        continue;
      }

      //---

      // line must have current tile in it
      line.sum = 0;

      int current = 0;

      for (int ix = line.start; ix <= line.end; ++ix) {
        CQQuintoTile *tile = cellTile(ix, iy);

        if (tile->turn() == turnInd)
          ++current;

        line.sum += tile->value();
      }

      if (! current)
        continue;

      line.current = current;

      //---

      hlines.push_back(line);
    }
  }

  // get vertical sequences
  for (int ix = 0; ix < nx; ++ix) {
    int iy = 0;

    while (iy < ny) {
      // find first tile
      while (iy < ny && ! cellTile(ix, iy))
        ++iy;

      if (iy >= ny)
        break;

      int iys = iy;

      // find last tile
      while (iy < ny && cellTile(ix, iy))
        ++iy;

      int iye = iy - 1;

      TileLine line(Direction::VERTICAL, iys, iye, ix);

      //---

      // ignore unit line
      if (line.len() == 1) {
        svline = line;
        continue;
      }

      //---

      // line must have current tile in it
      line.sum = 0;

      int current = 0;

      for (int iy = line.start; iy <= line.end; ++iy) {
        CQQuintoTile *tile = cellTile(ix, iy);

        if (tile->turn() == turnInd)
          ++current;

        line.sum += tile->value();
      }

      if (! current)
        continue;

      line.current = current;

      //---

      vlines.push_back(line);
    }
  }

  if (hlines.empty() && vlines.empty()) {
    CQQuintoTile *tile = cellTile(shline.start, shline.pos);

    shline.sum = tile->value();
    svline.sum = tile->value();

    hlines.push_back(shline);
    vlines.push_back(svline);
  }
}

int
CQQuintoBoard::
countTiles(int ix, int iy, Side side) const
{
  CQQuintoTile *tile = cellTile(ix, iy);

  if (! tile)
    return 0;

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  // count run to left, right, top or bottom
  int count = 1;

  if      (side == Side::LEFT) {
    --ix; while (ix >=      0 && cellTile(ix, iy)) { ++count; --ix; }
  }
  else if (side == Side::RIGHT) {
    ++ix; while (ix <= nx - 1 && cellTile(ix, iy)) { ++count; ++ix; }
  }
  else if (side == Side::TOP) {
    --iy; while (iy >=      0 && cellTile(ix, iy)) { ++count; --iy; }
  }
  else if (side == Side::BOTTOM) {
    ++iy; while (iy <= ny - 1 && cellTile(ix, iy)) { ++count; ++iy; }
  }

  return count;
}

void
CQQuintoBoard::
mousePressEvent(QMouseEvent *e)
{
  dragPos_  = e->globalPos();
  dragTile_ = nullptr;

  pressData_ = posToTileData(e->pos());

  CQQuintoPlayer *currentPlayer = quinto_->currentPlayer();

  int turnInd = quinto_->turn()->ind();

  if     (pressData_.owner == TileOwner::PLAYER1 || pressData_.owner == TileOwner::PLAYER2) {
    CQQuintoPlayer *pressPlayer = quinto_->player(pressData_.owner);

    if (pressPlayer == currentPlayer) {
      dragTile_ = currentPlayer->tile(pressData_.x);

      if (dragTile_) {
        QRect rect(dragPos_, QSize(playerTileSize(), playerTileSize()));

        dragTile_->show(rect);
      }
    }
  }
  else if (pressData_.owner == TileOwner::BOARD) {
    CQQuintoTile *tile = cellTile(pressData_.x, pressData_.y);

    if (tile && tile->turn() == turnInd) {
      dragTile_ = tile;

      QRect rect(dragPos_, QSize(boardTileSize(), boardTileSize()));

      dragTile_->show(rect);
    }
  }
}

void
CQQuintoBoard::
mouseMoveEvent(QMouseEvent *e)
{
  if (! dragTile_)
    return;

  QPoint dragPos = e->globalPos();

  dragTile_->move(dragPos);

  dragPos_ = dragPos;
}

void
CQQuintoBoard::
mouseReleaseEvent(QMouseEvent *e)
{
  if (! dragTile_)
    return;

  CQQuintoTile *dragTile = dragTile_;

  dragTile_ = nullptr;

  dragTile->hide();

  // player -> board
  if      (pressData_.owner == TileOwner::PLAYER1 || pressData_.owner == TileOwner::PLAYER2) {
    releaseData_ = posToTileData(e->pos());

    if (releaseData_.owner != TileOwner::BOARD)
      return;

    // ensure destination is empty (TODO: only to valid square)
    CQQuintoCell *cell = cells_[releaseData_.y][releaseData_.x];

    if (cell->tile())
      return;

    CQQuintoMove move(pressData_, releaseData_);

    quinto_->addMove(move);

    quinto_->doMove(move);
  }
  // board -> player or board
  else if (pressData_.owner == TileOwner::BOARD) {
    releaseData_ = posToTileData(e->pos());

    // board -> player
    if      (releaseData_.owner == TileOwner::PLAYER1 ||
             releaseData_.owner == TileOwner::PLAYER2) {
      CQQuintoPlayer *releasePlayer = quinto_->player(releaseData_.owner);

      CQQuintoPlayer *currentPlayer = quinto_->currentPlayer();

      if (releasePlayer != currentPlayer)
        return;

      CQQuintoMove move(pressData_, releaseData_);

      quinto_->addMove(move);

      quinto_->doMove(move);
    }
    else if (releaseData_.owner == TileOwner::BOARD) {
      CQQuintoCell *pressCell   = cells_[pressData_  .y][pressData_  .x];
      CQQuintoCell *releaseCell = cells_[releaseData_.y][releaseData_.x];

      CQQuintoTile *pressTile = pressCell->tile();
      assert(pressTile == dragTile);

      if (releaseCell->tile())
        return;

      CQQuintoMove move(pressData_, releaseData_);

      quinto_->addMove(move);

      quinto_->doMove(move);
    }
  }

  quinto_->updateState();
}

void
CQQuintoBoard::
keyPressEvent(QKeyEvent *ke)
{
  if      (ke->key() == Qt::Key_B)
    showBestMove();
  else if (ke->key() == Qt::Key_P)
    playBestMove();
}

CQQuintoBoard::TileData
CQQuintoBoard::
posToTileData(const QPoint &pos) const
{
  TileData tileData;

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  int x = pos.x();
  int y = pos.y();

  int ix = int((x - x_)/boardTileSize());
  int iy = int((y - y_)/boardTileSize());

  if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
    tileData.owner = TileOwner::BOARD;
    tileData.x     = ix;
    tileData.y     = iy;

    return tileData;
  }

  //---

  CQQuintoPlayer *player1 = quinto_->player1();
  CQQuintoPlayer *player2 = quinto_->player1();

  const int handSize = quinto_->handSize();

  iy = int((y - player1->tileY())/playerTileSize());

  if (iy == 0) {
    ix = int((x - player1->tileX())/playerTileSize());

    if (ix >= 0 && ix < handSize) {
      tileData.owner = TileOwner::PLAYER1;
      tileData.x     = ix;
      tileData.y     = iy;

      return tileData;
    }

    ix = int((x - player2->tileX())/playerTileSize());

    if (ix >= 0 && ix < handSize) {
      tileData.owner = TileOwner::PLAYER2;
      tileData.x     = ix;
      tileData.y     = iy;

      return tileData;
    }
  }

  return tileData;
}

//------

CQQuintoTurn::
CQQuintoTurn(CQQuinto *quinto, int ind) :
 quinto_(quinto), ind_(ind)
{
}

int
CQQuintoTurn::
score() const
{
  int score = 0;

  for (const auto &move : moves())
    score += quinto_->moveScore(move);

  return score;
}
