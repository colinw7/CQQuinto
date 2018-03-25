#include <CQQuinto.h>

#ifdef USE_QT_APP
#include <CQApp.h>
#else
#include <QApplication>
#endif

#include <CQPixmapCache.h>

#ifdef USE_HR_TIMER
#include <CHRTimer.h>
#endif

#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
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
#ifdef USE_QT_APP
  CQApp app(argc, argv);
#else
  QApplication app(argc, argv);
#endif

  auto seedRand = true;

  for (int i = 1; i < argc; ++i) {
    QString arg = argv[i];

    if (arg == "-noseed")
      seedRand = false;
  }

  if (seedRand)
    srand(time(nullptr));

  CQQuinto::App quinto;

  quinto.init();

  quinto.resize(quinto.sizeHint());

  quinto.show();

  return app.exec();
}

//---

namespace CQQuinto {

//------

App::
App(QWidget *parent) :
 QFrame(parent)
{
  setObjectName("quinto");
}

App::
~App()
{
  delete board_;

  for (auto &turn : turns_)
    delete turn;

  delete turn_;
}

void
App::
init()
{
  tileSet_ = std::make_unique<TileSet>(this);

  player1_ = std::make_unique<Player>(this, TileOwner::PLAYER1, "Player"  , PlayerType::HUMAN   );
  player2_ = std::make_unique<Player>(this, TileOwner::PLAYER2, "Computer", PlayerType::COMPUTER);

  currentPlayerOwner_ = TileOwner::PLAYER1;

  board_ = new Board(this);

  turn_ = new Turn(this, 0);

  //---

  createWidgets();

  //---

  player1_->drawTiles();
  player2_->drawTiles();

  //---

  updateState();
}

void
App::
createWidgets()
{
  auto createToolButton = [&](const QString &name, const QString &icon,
                              const char *slot) -> QToolButton* {
    auto button = new QToolButton(this);

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
                              const char *slot) -> QPushButton* {
    auto button = new QPushButton(this);

    button->setObjectName (name);
    button->setFocusPolicy(Qt::NoFocus);

    button->setText(text);

    connect(button, SIGNAL(clicked()), this, slot);

    return button;
  };

  newGameButton_ = createPushButton("newGame", "New Game", SLOT(newGameSlot()));

  //---

  auto createCombo = [&](const QString &name, const QStringList &items,
                         const char *slot) -> QComboBox* {
    auto combo = new QComboBox(this);

    combo->setObjectName (name);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFocusPolicy(Qt::NoFocus);

    combo->addItems(items);

    connect(combo, SIGNAL(currentIndexChanged(int)), this, slot);

    return combo;
  };

  modeCombo_ = createCombo("mode", QStringList() <<
    "Human/Human" << "Human/Computer" << "Computer/Human" << "Computer/Computer",
    SLOT(modeSlot(int)));

  modeCombo_->setCurrentIndex((int) playMode());
}

void
App::
resizeEvent(QResizeEvent *)
{
  updateWidgets();
}

void
App::
setPlayMode(PlayMode mode)
{
  if (mode != playMode_) {
    playMode_ = mode;

    if      (mode == PlayMode::HUMAN_COMPUTER) {
      player1_->setName("Player"  );
      player2_->setName("Computer");

      player1_->setType(PlayerType::HUMAN);
      player2_->setType(PlayerType::COMPUTER);
    }
    else if (mode == PlayMode::COMPUTER_HUMAN) {
      player1_->setName("Computer");
      player2_->setName("Player"  );

      player1_->setType(PlayerType::COMPUTER);
      player2_->setType(PlayerType::HUMAN);
    }
    else if (mode == PlayMode::HUMAN_HUMAN) {
      player1_->setName("Player1");
      player2_->setName("Player2");

      player1_->setType(PlayerType::HUMAN);
      player2_->setType(PlayerType::HUMAN);
    }
    else if (mode == PlayMode::COMPUTER_COMPUTER) {
      player1_->setName("Computer1");
      player2_->setName("Computer2");

      player1_->setType(PlayerType::COMPUTER);
      player2_->setType(PlayerType::COMPUTER);
    }
    else {
      assert(false);
    }

    newGame();
  }
}

void
App::
updateState()
{
  updateWidgets();

  update();

  //---

  //board_->showBestMove();
}

void
App::
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

  auto invalidateComboBoxSizeHint = [](QComboBox *w) {
    w->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    w->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  };

  //---

  double fs = calcFontScale(board_->playerTileSize());

  if (fs == lastFs_)
    return;

  //---

  const int b = 4;

  auto w = width ();
  auto h = height();

  //---

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

  auto bw = sh.width();

  cancelButton_->move(w - 1 - 3*bw - 2, 2);
  backButton_  ->move(w - 1 - 2*bw - 2, 2);
  applyButton_ ->move(w - 1 -   bw - 2, 2);

  //---

  newGameButton_->setFont(font);

  invalidatePushButtonSizeHint(newGameButton_);

  QSize nsh = newGameButton_->sizeHint();

  newGameButton_->resize(nsh);

  newGameButton_->move(w/2 - b - nsh.width(), h - 1 - nsh.height() - b);

  //---

  modeCombo_->setFont(font);

  invalidateComboBoxSizeHint(modeCombo_);

  QSize msh = modeCombo_->sizeHint();

  modeCombo_->move(w/2 + b, h - 1 - msh.height() - b);
}

void
App::
updateWidgets()
{
  const int b = 4;

  auto w = width ();
  auto h = height();

  //---

  board_->move(0, 0);

  board_->resize(w, h);

  board_->show();

  auto bw = backButton_->sizeHint().width();

  cancelButton_->move(w - 1 - 3*bw - 2, 2);
  backButton_  ->move(w - 1 - 2*bw - 2, 2);
  applyButton_ ->move(w - 1 -   bw - 2, 2);

  cancelButton_->show(); cancelButton_->raise();
  backButton_  ->show(); backButton_  ->raise();
  applyButton_ ->show(); applyButton_ ->raise();

  //---

  QSize nsh = newGameButton_->sizeHint();

  newGameButton_->move(w/2 - b - nsh.width(), h - 1 - nsh.height() - b);

  newGameButton_->show(); newGameButton_->raise();

  //---

  QSize msh = modeCombo_->sizeHint();

  modeCombo_->move(w/2 + b, h - 1 - msh.height() - b);

  modeCombo_->show(); modeCombo_->raise();

  //---

  auto validScore = isTurnValid();

  bool canUndo = ! turn_->moves().empty();

  cancelButton_->setEnabled(canUndo);
  backButton_  ->setEnabled(canUndo);
  applyButton_ ->setEnabled(validScore.valid);
}

//------

void
App::
cancelSlot()
{
  cancel();
}

void
App::
cancel()
{
  int n = turn_->moves().size();

  for (int i = n - 1; i >= 0; --i) {
    auto move = turn_->move(i);

    undoMove(move);
  }

  turn_->clear();

  //---

  updateState();
}

//------

void
App::
backSlot()
{
  back();
}

void
App::
back()
{
  int n = turn_->moves().size();

  if (n > 0) {
    auto move = turn_->move(n - 1);

    undoMove(move);

    turn_->popMove();
  }

  //---

  updateState();
}

//------

void
App::
applySlot()
{
  assert(currentPlayer()->type() == PlayerType::HUMAN);

  apply();
}

void
App::
apply(bool next)
{
  // check valid
  auto validScore = isTurnValid();

  if (! validScore.valid)
    return;

  assert((validScore.score % 5) == 0);

  // update score
  currentPlayer()->addScore(validScore.score);

  // get new tiles
  currentPlayer()->drawTiles();

  //---

  nextTurn();

  //---

  updateState();

  //---

  if (next)
    computerMove();
}

void
App::
computerMove()
{
  if (isGameOver())
    return;

  while (currentPlayer()->type() == PlayerType::COMPUTER) {
    auto currentPlayerOwner = this->currentPlayerOwner();

    while (currentPlayer()->owner() == currentPlayerOwner && currentPlayer()->canMove()) {
      // auto play computers best move
      playComputerMove();

      // if other player can move computer is done
      if (currentPlayer()->canMove())
        break;

      // skip other player
      nextTurn();

      assert(currentPlayer()->owner() == currentPlayerOwner);
    }

    // if computer can't move then check if game over
    if (currentPlayer()->owner() == currentPlayerOwner) {
      nextTurn();

      // if other player can't move then game over
      if (! currentPlayer()->canMove()) {
        setGameOver(true);
        break;
      }
    }
  }
}

//------

void
App::
newGameSlot()
{
  newGame();
}

void
App::
newGame()
{
  const int handSize = this->handSize();

  // move all board and player tiles back to tile set
  for (int i = 0; i < handSize; ++i) {
    auto tile1 = player1_->takeTile(i, /*nocheck*/true);
    auto tile2 = player2_->takeTile(i, /*nocheck*/true);

    if (tile1) tileSet_->ungetTile(tile1);
    if (tile2) tileSet_->ungetTile(tile2);
  }

  const int nx = this->nx();
  const int ny = this->ny();

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      TilePosition pos(ix, iy);

      if (board_->cellTile(pos)) {
        auto tile = board_->takeCellTile(pos);

        if (tile) tileSet_->ungetTile(tile);
      }
    }
  }

  //---

  // reset current player and player scores
  currentPlayerOwner_ = TileOwner::PLAYER1;

  player1_->setScore(0);
  player2_->setScore(0);

  player1_->setCanMove(true);
  player2_->setCanMove(true);

  //---

  // reset turns
  for (auto &turn : turns_)
    delete turn;

  turns_.clear();

  delete turn_;

  turn_ = new Turn(this, 0);

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

  //---

  computerMove();
}

//------

void
App::
modeSlot(int ind)
{
  if      (ind == 0) setPlayMode(PlayMode::HUMAN_HUMAN      );
  else if (ind == 1) setPlayMode(PlayMode::HUMAN_COMPUTER   );
  else if (ind == 2) setPlayMode(PlayMode::COMPUTER_HUMAN   );
  else if (ind == 3) setPlayMode(PlayMode::COMPUTER_COMPUTER);
  else               assert(false);
}

//------

void
App::
playComputerMove()
{
#ifdef USE_HR_TIMER
  //CScopeTimer timer("App::playComputerMove");
#endif

  assert(currentPlayer()->type() == PlayerType::COMPUTER);

  board_->playBestMove(false);

  //---

#ifdef USE_HR_TIMER
  //CIncrementalTimerMgrInst->clear();
#endif

  updateWidgets();

  qApp->processEvents();
}

void
App::
nextTurn()
{
  // next turn
  turns_.push_back(turn_);

  auto ind = turn_->ind() + 1;

  turn_ = new Turn(this, ind);

  //---

  // switch to next player
  currentPlayerOwner_ =
    (currentPlayerOwner_ == TileOwner::PLAYER1 ? TileOwner::PLAYER2 : TileOwner::PLAYER1);

  //---

  board_->invalidateDetails();
  board_->invalidateBestMove();

  //---

  currentPlayer()->setCanMove(canMove());
}

void
App::
setGameOver(bool b)
{
  gameOver_ = b;

  if (gameOver_)
    newGameButton_->setText("Game Over");
  else
    newGameButton_->setText("New Game");

  updateState();
}

bool
App::
canMove() const
{
#ifdef USE_HR_TIMER
  //CScopeTimer timer("App::canMove");
#endif

  if (currentPlayer()->numTiles() == 0)
    return false;

  auto details = board_->boardDetails();

  if (! details.valid)
    return false;

  if (details.validPositions.empty())
    return false;

  const BestMove &bestMove = board_->getBestMove();

  if (! bestMove.isValid())
    return false;

  return true;
}

ValidScore
App::
isTurnValid() const
{
  auto details = board_->boardDetails();

  ValidScore validScore;

  validScore.valid = details.valid && ! details.partial;
  validScore.score = details.score;

  return validScore;
}

void
App::
undoMove(const Move &move)
{
  doMoveParts(move.to(), move.from());
}

void
App::
doMove(const Move &move)
{
  doMoveParts(move.from(), move.to());
}

void
App::
doMoveParts(const TileData &from, const TileData &to)
{
  if      (from.owner == TileOwner::PLAYER1) {
    assert(to.owner == TileOwner::BOARD);

    auto fromTile = player1_->takeTile(from.pos.ix);
    assert(fromTile);

    fromTile->setPlayer(from.owner);
    fromTile->setTurn  (turn()->ind());

    board_->setCellTile(to.pos, fromTile);
  }
  else if (from.owner == TileOwner::PLAYER2) {
    assert(to.owner == TileOwner::BOARD);

    auto fromTile = player2_->takeTile(from.pos.ix);
    assert(fromTile);

    fromTile->setPlayer(from.owner);
    fromTile->setTurn  (turn()->ind());

    board_->setCellTile(to.pos, fromTile);
  }
  else if (from.owner == TileOwner::BOARD) {
    if      (to.owner == TileOwner::PLAYER1 ||
             to.owner == TileOwner::PLAYER2) {
      auto fromTile = board_->takeCellTile(from.pos);
      assert(fromTile);

      if (to.owner == TileOwner::PLAYER1)
        player1_->addTile(fromTile, to.pos.ix);
      else
        player2_->addTile(fromTile, to.pos.ix);
    }
    else if (to.owner == TileOwner::BOARD) {
      auto fromCell = board_->takeCellTile(from.pos);
      assert(fromCell);

      board_->setCellTile(to.pos, fromCell);
    }
  }
  else {
    assert(false);
  }
}

int
App::
moveScore(const Move &move) const
{
  auto to = move.to();

  if (to.owner == TileOwner::BOARD) {
    auto tile = board_->cellTile(to.pos);

    if (tile)
      return tile->value();
  }

  return 0;
}

void
App::
addMove(const Move &move)
{
  turn_->addMove(move);
}

double
App::
calcFontScale(double s) const
{
  QFont font;

  QFontMetricsF fm(font);

  double w = fm.width("8");
  double h = fm.height();

  return s/std::max(w, h);
}

QSize
App::
sizeHint() const
{
  return QSize(1200, 1200);
}

//---

TileData::
TileData(TileOwner owner, const TilePosition &pos) :
 owner(owner), pos(pos)
{
}

void
TileData::
print(std::ostream &os) const
{
  if      (owner == TileOwner::PLAYER1 ) { os << "Player1:" << pos.ix; }
  else if (owner == TileOwner::PLAYER2 ) { os << "Player2:" << pos.ix; }
  else if (owner == TileOwner::BOARD   ) { os << "Board:" << pos.ix << "," << pos.iy; }
  else if (owner == TileOwner::TILE_SET) { os << "TileSet:"; }
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

TileSet::
TileSet(App *quinto) :
 quinto_(quinto)
{
  auto addTile = [&](int value) {
    auto ind = tiles_.size();

    auto tile = new Tile(quinto_, ind, value);

    tile->setOwner (TileOwner::TILE_SET);
    tile->setPlayer(TileOwner::NONE);

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

TileSet::
~TileSet()
{
  for (auto &tile : itiles_)
    delete tile;
}

void
TileSet::
shuffle()
{
  auto nt = tiles_.size();

  auto ns = 100;

  for (auto i = 0; i < ns; ++i) {
    auto pos1 = rand() % nt;
    auto pos2 = rand() % nt;

    while (pos1 == pos2)
      pos2 = rand() % nt;

    std::swap(tiles_[pos1], tiles_[pos2]);
  }
}

Tile *
TileSet::
getTile()
{
  if (tiles_.empty())
    return nullptr;

  auto tile = tiles_.back();

  tile->setOwner (TileOwner::NONE);
  tile->setPlayer(TileOwner::NONE);

  tiles_.pop_back();

  return tile;
}

void
TileSet::
ungetTile(Tile *tile)
{
  tile->setOwner (TileOwner::TILE_SET);
  tile->setPlayer(TileOwner::NONE);

  tiles_.push_back(tile);
}

//---

Tile::
Tile(App *quinto, int ind, int value) :
 quinto_(quinto), ind_(ind), value_(value)
{
  setObjectName(QString("tile.%1").arg(ind_));

  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);

  hide();
}

void
Tile::
show(const QRect &rect)
{
  resize(rect.size());

  move(rect.topLeft());

  QWidget::show();
}

void
Tile::
hide()
{
  setVisible(false);
}

void
Tile::
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
Tile::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);

  drawTile(&painter);
}

void
Tile::
drawTile(QPainter *painter)
{
  QColor fg = quinto_->tileBorderColor();

  painter->setPen(fg);

  drawTile(painter, rect());
}

void
Tile::
drawTile(QPainter *painter, const QRectF &rect)
{
  QColor textColor = painter->pen().color();

  //---

  QColor fg = quinto_->tileBorderColor();

  painter->setPen(fg);

  painter->drawRect(rect);

  //---

  painter->setFont(font_);

  QFontMetricsF fm(painter->font());

  auto value = this->value();

  QString text = QString("%1").arg(value);

  double tx = rect.center().x() - fm.width(text)/2.0;
  double ty = rect.center().y() + (fm.ascent() - fm.descent())/2;

  painter->setPen(textColor);

  painter->drawText(QPointF(tx, ty), text);
}

//---

Player::
Player(App *quinto, TileOwner owner, const QString &name, PlayerType type) :
 quinto_(quinto), owner_(owner), name_(name), type_(type)
{
  const int handSize = quinto_->handSize();

  for (int i = 0; i < handSize; ++i)
    tiles_.push_back(nullptr);
}

int
Player::
numTiles() const
{
  const int handSize = quinto_->handSize();

  auto n = 0;

  for (int i = 0; i < handSize; ++i)
    if (tiles_[i])
      ++n;

  return n;
}

void
Player::
drawTiles()
{
  int nt = tiles_.size();

  for (int i = 0; i < nt; ++i) {
    if (tiles_[i])
      continue;

    auto tile = quinto_->tileSet()->getTile();

    if (! tile)
      break;

    tile->setOwner (owner());
    tile->setPlayer(TileOwner::NONE);

    tiles_[i] = tile;
  }
}

Tile *
Player::
tile(int i) const
{
  //assert(i >= 0 && i < quinto_->handSize());

  return tiles_[i];
}

Tile *
Player::
takeTile(int i, bool nocheck)
{
  //assert(i >= 0 && i < quinto_->handSize());

  auto tile = tiles_[i];

  if (nocheck) {
    if (! tile)
      return nullptr;
  }
  else {
    assert(tile);
  }

  tile->setOwner (TileOwner::NONE);
  tile->setPlayer(TileOwner::NONE);

  tiles_[i] = nullptr;

  return tile;
}

void
Player::
addTile(Tile *tile, int i)
{
  //assert(i >= 0 && i < quinto_->handSize());

  //assert(! tiles_[i]);

  tiles_[i] = tile;

  tile->setOwner (owner());
  tile->setPlayer(TileOwner::NONE);
}

//---

Board::
Board(App *quinto) :
 QWidget(quinto), quinto_(quinto)
{
  setObjectName("board");

  setFocusPolicy(Qt::StrongFocus);

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  tiles_.resize(ny);

  for (int iy = 0; iy < ny; ++iy) {
    tiles_[iy].resize(nx);

    for (int ix = 0; ix < nx; ++ix)
      tiles_[iy][ix] = nullptr;
  }
}

#if 0
Tile *
Board::
cellTile(const TilePosition &pos) const
{
  assert(validPos(pos));

  return tiles_[pos.iy][pos.ix];
}
#endif

Tile *
Board::
takeCellTile(const TilePosition &pos)
{
  //assert(validPos(pos));

  auto tile = tiles_[pos.iy][pos.ix];
  assert(tile);

  tiles_[pos.iy][pos.ix] = nullptr;

  tile->setOwner (TileOwner::NONE);
  tile->setPlayer(TileOwner::NONE);

  invalidateDetails();
  invalidateBestMove();

  return tile;
}

void
Board::
setCellTile(const TilePosition &pos, Tile *tile)
{
  //assert(validPos(pos));

  assert(! tiles_[pos.iy][pos.ix]);

  tile->setOwner(TileOwner::BOARD);

  tiles_[pos.iy][pos.ix] = tile;

  invalidateDetails();
  invalidateBestMove();
}

void
Board::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);

  drawBoard(&painter);

  quinto_->updateFonts();
}

void
Board::
drawBoard(QPainter *painter)
{
  const int b = 4;

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  //---

  double w = width () - 2*b;
  double h = height() - 4*b;

  double ts = boardTileSize();

  auto ny1 = ny + 2; // +2 for player tiles (top) and score (bottom)

  bs_ = std::min(w/nx, h/ny1);

  pos_ = QPointF((w - nx *ts)/2 + b, (h - ny1*ts)/2 + 2*b + ts);

  ps_ = ts*0.6;

  //----

  // draw turn
  drawTurn(painter);

  //----

  // draw player tiles
  const PlayerP &player1 = quinto_->player1();
  const PlayerP &player2 = quinto_->player2();

  const PlayerP &currentPlayer = quinto_->currentPlayer();

  if      (quinto_->playMode() == PlayMode::HUMAN_COMPUTER) {
    if (currentPlayer == player1 && player1->type() == PlayerType::HUMAN)
      drawPlayerTiles(painter, player1, width()/2, Qt::AlignHCenter);
  }
  else if (quinto_->playMode() == PlayMode::COMPUTER_HUMAN) {
    if (currentPlayer == player2 && player2->type() == PlayerType::HUMAN)
      drawPlayerTiles(painter, player2, width()/2, Qt::AlignHCenter);
  }
  else if (quinto_->playMode() == PlayMode::HUMAN_HUMAN) {
    drawPlayerTiles(painter, player1, width()/2, Qt::AlignRight);
    drawPlayerTiles(painter, player2, width()/2, Qt::AlignLeft );
  }
  else if (quinto_->playMode() == PlayMode::COMPUTER_COMPUTER) {
    drawPlayerTiles(painter, player1, width()/2, Qt::AlignRight);
    drawPlayerTiles(painter, player2, width()/2, Qt::AlignLeft );
  }
  else {
    assert(false);
  }

  //----

  auto details = boardDetails();

  //----

  // draw board tiles
  auto turnInd = quinto_->turn()->ind();

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      TilePosition pos(ix, iy);

      QRectF rect(pos_.x() + ix*ts, pos_.y() + iy*ts, ts, ts);

      auto tile = this->cellTile(pos);

      bool current = (tile && tile->turn() == turnInd);
      bool valid   = (details.validPositions.find(pos) != details.validPositions.end());

      QColor bgColor;
      QColor fgColor = quinto_->tileBorderColor();

      if (tile) {
        bgColor = quinto_->tileBgColor();

        if (current) {
          if (! details.valid)
            fgColor = quinto_->invalidTileColor();
          else
            fgColor = quinto_->currentTileColor();
        }
        else {
          if (tile->player() == TileOwner::PLAYER1)
            fgColor = quinto_->player1TileColor();
          else
            fgColor = quinto_->player2TileColor();
        }
      }
      else if (valid) {
        bgColor = quinto_->validMoveColor();
        fgColor = Qt::black;
      }
      else {
        bgColor = Qt::white;
        fgColor = Qt::black;
      }

      drawTile(painter, tile, rect, ts, bgColor, fgColor);
    }
  }

  //---

  drawScores(painter);
}

void
Board::
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

  auto dbt = playerTileSize() - fm.height();

  double x = b;
  double y = b + dbt/2 + fm.ascent();

  painter->drawText(x, y, turnText);
}

void
Board::
drawScores(QPainter *painter)
{
  const int b = 4;

  const PlayerP &player1 = quinto_->player1();
  const PlayerP &player2 = quinto_->player2();

  // set font
  QFont font;

  double fs = quinto_->calcFontScale(playerTileSize());

  font.setPointSizeF(font.pointSizeF()*fs);

  painter->setFont(font);

  QFontMetricsF fm(painter->font());

  //---

  // draw scores
  auto p1score = player1->score();
  auto p1extra = 0;

  auto p2score = player2->score();
  auto p2extra = 0;

  QString score1Text = QString("%1").arg(p1score);
  QString score2Text = QString("%1").arg(p2score);

  if (details_.valid) {
    const PlayerP &currentPlayer = quinto_->currentPlayer();

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

  auto sy = height() - 1 - fm.descent() - 2;

  // draw player 1 score
  auto sx1 = 2*b;

  painter->drawText(sx1, sy, p1Title);

  sx1 += fm.width(p1Title) + 2*b;

  painter->drawText(sx1, sy, score1Text);

  // draw player 2 score
  auto sx2 = width() - 2*b - fm.width(score2Text);

  painter->drawText(sx2, sy, score2Text);

  sx2 -= fm.width(p2Title) + 2*b;

  painter->drawText(sx2, sy, p2Title);
}

void
Board::
drawPlayerTiles(QPainter *painter, const PlayerP &player, int x, Qt::Alignment align)
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
    px = x + 2*b;
    tx = px + handSize*playerTileSize() + b;
  }
  else if (align == Qt::AlignRight) {
    px = x - 2*b - handSize*playerTileSize();
    tx = px - b - fm.width(playerText);
  }
  else {
    tx = x - fm.width(playerText)/2 - handSize*playerTileSize()/2;
    px = tx + fm.width(playerText) + 2*b;
  }

  auto ty = b;

  player->setTileX(px);
  player->setTileY(ty);

  //---

  // draw player name
  if (quinto_->playMode() == PlayMode::HUMAN_HUMAN) {
    const PlayerP &currentPlayer = quinto_->currentPlayer();

    painter->setPen(player == currentPlayer ? quinto_->currentPlayerColor() : Qt::black);
  }
  else
    painter->setPen(Qt::black);

  painter->setFont(font);

  auto dbt = playerTileSize() - fm.height();

  painter->drawText(tx, player->tileY() + dbt/2 + fm.ascent(), playerText);

  //---

  // draw player tiles
  QColor bgColor = quinto_->tileBgColor();
  QColor fgColor = quinto_->tileBorderColor();

  if (player->owner() == TileOwner::PLAYER1)
    fgColor = quinto_->player1TileColor();
  else
    fgColor = quinto_->player2TileColor();

  for (int i = 0; i < handSize; ++i) {
    double x = px + i*playerTileSize();

    QRectF rect(x, player->tileY(), playerTileSize(), playerTileSize());

    auto tile = player->tile(i);

    drawTile(painter, tile, rect, playerTileSize(), bgColor, fgColor);
  }
}

void
Board::
drawTile(QPainter *painter, Tile *tile, const QRectF &rect, double s,
         const QColor &bgColor, const QColor &fgColor)
{
  painter->setBrush(bgColor);
  painter->setPen  (fgColor);

  if (tile) {
    tile->setSize(s);

    tile->drawTile(painter, rect);
  }
  else {
    QColor fg = quinto_->tileBorderColor();

    painter->setPen(fg);

    painter->drawRect(rect);
  }
}

void
Board::
playBestMove(bool next)
{
  const BestMove &bestMove = getBestMove();

  if (! bestMove.isValid())
    return; // assert ?

  for (const auto &move : bestMove.moves)
    quinto_->doMove(move);

  quinto_->apply(next);
}

void
Board::
showBestMove() const
{
  const BestMove &bestMove = getBestMove();

  if (! bestMove.isValid())
    return; // assert ?

  std::cerr << "Best Moves:";

  for (const auto &move : bestMove.moves) {
    std::cerr << " ";

    move.print(std::cerr);
  }

  std::cerr << " @" << bestMove.score << "\n";
}

const BestMove &
Board::
getBestMove() const
{
  if (! bestMoveValid_) {
    auto th = const_cast<Board *>(this);

    th->calcBestMove();

    th->bestMoveValid_ = true;
  }

  return bestMove_;
}

void
Board::
calcBestMove()
{
  bestMove_.reset();

  auto moveTree = boardMoveTree();

  if (! moveTree)
    return;

  //std::cerr << "Move Tree: "; moveTree->print(std::cerr); std::cerr << "\n";

  auto maxLeaf = moveTree->maxLeaf();

  if (maxLeaf) {
    maxLeaf->hierMoves(bestMove_.moves);

    bestMove_.score = maxLeaf->score;
  }

  //---

  delete moveTree;
}

MoveTree *
Board::
boardMoveTree() const
{
  auto root = new MoveTree;

  (void) buildMoveTree(root, 0);

  return root;
}

bool
Board::
buildMoveTree(MoveTree *tree, int depth) const
{
  assert(depth <= 5);

  BoardMoves moves;

  moves.depth = depth;

  if (! boardMoves(moves))
    return false;

  tree->partial = moves.partial;
  tree->score   = moves.score;

  for (auto &move : moves.moves) {
    quinto_->doMoveParts(move.from(), move.to());

    auto child = new MoveTree;

    if (buildMoveTree(child, depth + 1)) {
      tree->addChild(child);

      child->move = move;
    }
    else {
      delete child;
    }

    quinto_->doMoveParts(move.to(), move.from());
  }

std::cerr << tree->root()->size() << std::endl;
  return true;
}

bool
Board::
boardMoves(BoardMoves &moves) const
{
  auto details = boardDetails();

  if (! details.valid)
    return false;

  //assert(details.npt == moves.depth);

  //---

  moves.score   = details.score;
  moves.partial = details.partial;

  //---

  const int handSize = quinto_->handSize();

  const PlayerP &currentPlayer = quinto_->currentPlayer();

  auto playerOwner = currentPlayer->owner();

  for (const auto &position : details.validPositions) {
    using ValueSet = std::set<int>;

    ValueSet values;

    for (int i = 0; i < handSize; ++i) {
      auto tile = currentPlayer->tile(i);
      if (! tile) continue;

      auto p = values.find(tile->value());
      if (p != values.end()) continue;

      TilePosition pos(i, 0);

      TileData from(playerOwner, pos);
      TileData to  (TileOwner::BOARD, position);

      Move move(from, to);

      moves.moves.push_back(move);

      values.insert(tile->value());
    }
  }

  return true;
}

const BoardDetails &
Board::
boardDetails() const
{
  if (! detailsValid_) {
    auto th = const_cast<Board *>(this);

    th->calcBoardDetails();

    th->detailsValid_ = true;
  }

  return details_;
}

// depends on cells, current turn
void
Board::
calcBoardDetails()
{
  auto addValidPosition = [&](const TilePosition &pos) {
    //assert(! cellTile(pos));

    details_.addValidPosition(pos);
  };

  //---

  details_.reset();

  details_.valid   = true;
  details_.partial = false;

  //---

  auto turnInd = quinto_->turn()->ind();

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  //---

  // count number of current move tiles in each row (y) and column (x) and
  // save row/columns containing current move tiles
  BoardLines boardLines;

  details_.nt  = 0;
  details_.npt = 0;

  for (int iy = 0; iy < ny; ++iy) {
    for (int ix = 0; ix < nx; ++ix) {
      TilePosition pos(ix, iy);

      auto tile = cellTile(pos);
      if (! tile) continue;

      ++details_.nt;

      if (tile->turn() == turnInd) {
        boardLines.xinds.insert(ix);
        boardLines.yinds.insert(iy);

        ++details_.npt;
      }
    }
  }

  //---

  // if board empty then must be valid
  // return center positions for valid positions
  if (details_.nt == 0) {
#if 0
    // multiple center positions
    auto ix1 = (nx - 1)/2, ix2 = nx/2;
    auto iy1 = (ny - 1)/2, iy2 = ny/2;

    addValidPosition(TilePosition(ix1, iy1));
    addValidPosition(TilePosition(ix2, iy1));
    addValidPosition(TilePosition(ix1, iy2));
    addValidPosition(TilePosition(ix2, iy2));
#else
    // single center position
    auto ix1 = (nx - 1)/2;
    auto iy1 = (ny - 1)/2;

    addValidPosition(TilePosition(ix1, iy1));
#endif

    details_.partial = true;

    return;
  }

  //----

  // no tiles placed yet (for current player) then must be valid,
  if (details_.npt == 0) {
    //assert(boardLines.xinds.empty() && boardLines.yinds.empty());

    // play off existing pieces (board not empty)
    for (int iy = 0; iy < ny; ++iy) {
      for (int ix = 0; ix < nx; ++ix) {
        TilePosition pos(ix, iy);

        auto tile = cellTile(pos);
        if (tile) continue;

        // check if empty cell has any surrounding tiles
        TilePosition l_pos = pos.left  ();
        TilePosition r_pos = pos.right ();
        TilePosition t_pos = pos.top   ();
        TilePosition b_pos = pos.bottom();

        auto l_tile = (ix > 0      ? cellTile(l_pos) : nullptr);
        auto r_tile = (ix < nx - 1 ? cellTile(r_pos) : nullptr);
        auto t_tile = (iy > 0      ? cellTile(t_pos) : nullptr);
        auto b_tile = (iy < ny - 1 ? cellTile(b_pos) : nullptr);

        if (! l_tile && ! r_tile && ! t_tile && ! b_tile)
          continue;

        //---

        // count run to left, right, top, bottom
        auto l_count = (l_tile ? countTiles(l_pos, Side::LEFT  ) : 0);
        auto r_count = (r_tile ? countTiles(r_pos, Side::RIGHT ) : 0);

        if (l_count + r_count + 1 > 5)
          continue;

        auto t_count = (t_tile ? countTiles(t_pos, Side::TOP   ) : 0);
        auto b_count = (b_tile ? countTiles(b_pos, Side::BOTTOM) : 0);

        if (t_count + b_count + 1 > 5)
          continue;

        //---

        addValidPosition(pos);
      }
    }

    // can't apply yet
    details_.partial = true;

    return;
  }

  //---

  // get connected lines, length 2 or more, including at least one turn piece
  getBoardLines(boardLines);

  //---

  // check all lines
  for (const auto &line : boardLines.hlines) {
    LineValid lineValid;

    details_.valid = line.isValid(lineValid);

    details_.partial = lineValid.partial;
    details_.errMsg  = lineValid.errMsg;

    if (! details_.valid) {
      //line.print(std::cerr, details_.errMsg); std::cerr << "\n";
      return;
    }
  }

  for (const auto &line : boardLines.vlines) {
    LineValid lineValid;

    details_.valid = line.isValid(lineValid);

    details_.partial = lineValid.partial;
    details_.errMsg  = lineValid.errMsg;

    //line.print(std::cerr, details_.errMsg); std::cerr << "\n";

    if (! details_.valid)
      return;
  }

  //---

  // single piece played then check row or column
  if (details_.npt == 1) {
    //assert(boardLines.xinds.size() == 1 && boardLines.yinds.size() == 1);

    auto ix1 = *boardLines.xinds.begin();
    auto iy1 = *boardLines.yinds.begin();

    //---

    // play off vertical lines of existing piece (board not empty)
    for (int iy = 0; iy < ny; ++iy) {
      TilePosition pos(ix1, iy);

      auto tile = cellTile(pos);
      if (tile) continue;

      // check if empty cell has any surrounding tiles
      TilePosition t_pos = pos.top   ();
      TilePosition b_pos = pos.bottom();

      auto t_tile = (iy > 0      ? cellTile(t_pos) : nullptr);
      auto b_tile = (iy < ny - 1 ? cellTile(b_pos) : nullptr);

      if (! t_tile && ! b_tile)
        continue;

      //---

      // count runs at top and bottom
      auto t_count = (t_tile ? countTiles(t_pos, Side::TOP   ) : 0);
      auto b_count = (b_tile ? countTiles(b_pos, Side::BOTTOM) : 0);

      if (t_count + b_count + 1 > 5)
        continue;

      //---

      addValidPosition(pos);
    }

    //---

    // play off horizontal lines of existing piece (board not empty)
    for (int ix = 0; ix < nx; ++ix) {
      TilePosition pos(ix, iy1);

      auto tile = cellTile(pos);
      if (tile) continue;

      // check if empty cell has any surrounding tiles
      TilePosition l_pos = pos.left ();
      TilePosition r_pos = pos.right();

      auto l_tile = (ix > 0      ? cellTile(l_pos) : nullptr);
      auto r_tile = (ix < nx - 1 ? cellTile(r_pos) : nullptr);

      if (! l_tile && ! r_tile)
        continue;

      //---

      // count runs to left and right
      auto l_count = (l_tile ? countTiles(l_pos, Side::LEFT  ) : 0);
      auto r_count = (r_tile ? countTiles(r_pos, Side::RIGHT ) : 0);

      if (l_count + r_count + 1 > 5)
        continue;

      //---

      addValidPosition(pos);
    }

    //---

    //TilePosition pos(ix1, iy1);

    //auto tile = cellTile(pos);
    //assert(tile);

    details_.valid   = true;
    details_.score   = boardLines.score();
    details_.partial = ((details_.score % 5) != 0);

    return;
  }

  //---

  // two or more pieces. must be in a single row or column
  if (boardLines.xinds.size() > 1 && boardLines.yinds.size() > 1) {
    details_.valid  = false;
    details_.errMsg = "Disjoint pieces";
    return;
  }

  //---

  bool horizontal = (boardLines.xinds.size() > 1);

  //---

  TileLines &lines = (horizontal ? boardLines.hlines : boardLines.vlines);
  //assert(! lines.empty());

  //---

  // score all lines
  details_.score = boardLines.score();

  //---

  details_.partial = ((details_.score % 5) != 0);

  //---

  // add valid positions (end of lines)
  for (const auto &line : lines) {
    if (line.len() >= 5)
      continue;

    if (horizontal) {
      auto ix1 = line.start - 1;
      auto ix2 = line.end   + 1;

      if (ix1 >= 0 ) addValidPosition(TilePosition(ix1, line.pos));
      if (ix2 <  nx) addValidPosition(TilePosition(ix2, line.pos));
    }
    else {
      auto iy1 = line.start - 1;
      auto iy2 = line.end   + 1;

      if (iy1 >= 0 ) addValidPosition(TilePosition(line.pos, iy1));
      if (iy2 <  ny) addValidPosition(TilePosition(line.pos, iy2));
    }
  }

  //---

  if (! details_.partial) {
    //assert((details_.score % 5) == 0);
  }
}

void
Board::
getBoardLines(BoardLines &boardLines) const
{
#ifdef USE_HR_TIMER
  //CIncrementalTimer *timer = CIncrementalTimerMgrInst->get("Board::getBoardLines");
  //CIncrementalTimerScope stimer(timer);
#endif

  //---

  TileLine shline, svline;

  auto turnInd = quinto_->turn()->ind();

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  // get horizontal sequence
  for (const auto &iy : boardLines.yinds) {
    auto ix = 0;

    while (ix < nx) {
      // find first tile
      while (ix < nx && ! cellTile(TilePosition(ix, iy)))
        ++ix;

      if (ix >= nx)
        break;

      auto ixs = ix;

      // find last tile
      while (ix < nx && cellTile(TilePosition(ix, iy)))
        ++ix;

      auto ixe = ix - 1;

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

      auto current = 0;

      for (int ix = line.start; ix <= line.end; ++ix) {
        auto tile = cellTile(TilePosition(ix, iy));

        if (tile->turn() == turnInd)
          ++current;

        line.sum += tile->value();
      }

      if (! current)
        continue;

      line.current = current;

      //---

      boardLines.hlines.push_back(line);
    }
  }

  // get vertical sequences
  for (const auto &ix : boardLines.xinds) {
    auto iy = 0;

    while (iy < ny) {
      // find first tile
      while (iy < ny && ! cellTile(TilePosition(ix, iy)))
        ++iy;

      if (iy >= ny)
        break;

      auto iys = iy;

      // find last tile
      while (iy < ny && cellTile(TilePosition(ix, iy)))
        ++iy;

      auto iye = iy - 1;

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

      auto current = 0;

      for (int iy = line.start; iy <= line.end; ++iy) {
        auto tile = cellTile(TilePosition(ix, iy));

        if (tile->turn() == turnInd)
          ++current;

        line.sum += tile->value();
      }

      if (! current)
        continue;

      line.current = current;

      //---

      //assert(boardLines.xinds.find(ix) != boardLines.xinds.end());

      boardLines.vlines.push_back(line);
    }
  }

  if (boardLines.hlines.empty() && boardLines.vlines.empty()) {
    auto tile = cellTile(TilePosition(shline.start, shline.pos));

    shline.sum = tile->value();
    svline.sum = tile->value();

    boardLines.hlines.push_back(shline);
    boardLines.vlines.push_back(svline);
  }
}

int
Board::
countTiles(const TilePosition &pos, Side side) const
{
  //assert(cellTile(pos));

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  // count run to left, right, top or bottom
  auto count = 1;

  auto ix = pos.ix;
  auto iy = pos.iy;

  if      (side == Side::LEFT) {
    --ix; while (ix >=      0 && cellTile(TilePosition(ix, iy))) { ++count; --ix; }
  }
  else if (side == Side::RIGHT) {
    ++ix; while (ix <= nx - 1 && cellTile(TilePosition(ix, iy))) { ++count; ++ix; }
  }
  else if (side == Side::TOP) {
    --iy; while (iy >=      0 && cellTile(TilePosition(ix, iy))) { ++count; --iy; }
  }
  else if (side == Side::BOTTOM) {
    ++iy; while (iy <= ny - 1 && cellTile(TilePosition(ix, iy))) { ++count; ++iy; }
  }

  return count;
}

void
Board::
mousePressEvent(QMouseEvent *e)
{
  dragPos_  = e->globalPos();
  dragTile_ = nullptr;

  pressData_ = posToTileData(e->pos());

  const PlayerP &currentPlayer = quinto_->currentPlayer();

  auto turnInd = quinto_->turn()->ind();

  if     (pressData_.owner == TileOwner::PLAYER1 ||
          pressData_.owner == TileOwner::PLAYER2) {
    const PlayerP &pressPlayer = quinto_->ownerPlayer(pressData_.owner);

    if (pressPlayer == currentPlayer) {
      dragTile_ = currentPlayer->tile(pressData_.pos.ix);

      if (dragTile_) {
        QRect rect(dragPos_, QSize(playerTileSize(), playerTileSize()));

        dragTile_->show(rect);
      }
    }
  }
  else if (pressData_.owner == TileOwner::BOARD) {
    TilePosition pressPos = pressData_.pos;

    auto tile = cellTile(pressPos);

    if (tile && tile->turn() == turnInd) {
      dragTile_ = tile;

      double ts = boardTileSize();

      QRect rect(dragPos_, QSize(ts, ts));

      dragTile_->show(rect);
    }
  }
}

void
Board::
mouseMoveEvent(QMouseEvent *e)
{
  if (! dragTile_)
    return;

  auto dragPos = e->globalPos();

  dragTile_->move(dragPos);

  dragPos_ = dragPos;
}

void
Board::
mouseReleaseEvent(QMouseEvent *e)
{
  if (! dragTile_)
    return;

  auto dragTile = dragTile_;

  dragTile_ = nullptr;

  dragTile->hide();

  releaseData_ = posToTileData(e->pos());

  // player -> board
  if      (pressData_.owner == TileOwner::PLAYER1 ||
           pressData_.owner == TileOwner::PLAYER2) {
    if (releaseData_.owner != TileOwner::BOARD)
      return;

    // ensure destination is empty (TODO: only to valid square)
    auto tile = this->cellTile(releaseData_.pos);

    if (tile)
      return;
  }
  // board -> player or board
  else if (pressData_.owner == TileOwner::BOARD) {
    // board -> player
    if      (releaseData_.owner == TileOwner::PLAYER1 ||
             releaseData_.owner == TileOwner::PLAYER2) {
      const PlayerP &releasePlayer = quinto_->ownerPlayer(releaseData_.owner);

      const PlayerP &currentPlayer = quinto_->currentPlayer();

      if (releasePlayer != currentPlayer)
        return;
    }
    else if (releaseData_.owner == TileOwner::BOARD) {
      auto pressTile = this->cellTile(pressData_.pos);
      assert(pressTile == dragTile);

      auto releaseTile = this->cellTile(releaseData_.pos);

      if (releaseTile)
        return;
    }
    else
      return;
  }
  else {
    return;
  }

  //---

  Move move(pressData_, releaseData_);

  quinto_->addMove(move);

  quinto_->doMove(move);

  quinto_->updateState();
}

void
Board::
keyPressEvent(QKeyEvent *ke)
{
  if      (ke->key() == Qt::Key_B)
    showBestMove();
  else if (ke->key() == Qt::Key_P)
    playBestMove();
}

TileData
Board::
posToTileData(const QPoint &pos) const
{
  TileData tileData;

  const int nx = quinto_->nx();
  const int ny = quinto_->ny();

  auto x = pos.x();
  auto y = pos.y();

  double ts = boardTileSize();

  auto ix = int((x - pos_.x())/ts);
  auto iy = int((y - pos_.y())/ts);

  if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
    tileData.owner = TileOwner::BOARD;
    tileData.pos   = TilePosition(ix, iy);

    return tileData;
  }

  //---

  const PlayerP &player1 = quinto_->player1();
  const PlayerP &player2 = quinto_->player2();

  const int handSize = quinto_->handSize();

  iy = int((y - player1->tileY())/playerTileSize());

  if (iy == 0) {
    if (quinto_->playMode() == PlayMode::HUMAN_COMPUTER ||
        quinto_->playMode() == PlayMode::HUMAN_HUMAN) {
      ix = int((x - player1->tileX())/playerTileSize());

      if (ix >= 0 && ix < handSize) {
        tileData.owner = TileOwner::PLAYER1;
        tileData.pos   = TilePosition(ix, iy);

        return tileData;
      }
    }

    if (quinto_->playMode() == PlayMode::COMPUTER_HUMAN ||
        quinto_->playMode() == PlayMode::HUMAN_HUMAN) {
      ix = int((x - player2->tileX())/playerTileSize());

      if (ix >= 0 && ix < handSize) {
        tileData.owner = TileOwner::PLAYER2;
        tileData.pos   = TilePosition(ix, iy);

        return tileData;
      }
    }
  }

  return tileData;
}

//------

int
BoardLines::
score() const
{
  // score all lines
  int score = 0;

  for (const auto &line : hlines)
    score += line.sum;

  for (const auto &line : vlines)
    score += line.sum;

  return score;
}

//------

TileLine::
TileLine(Direction direction, int start, int end, int pos) :
 direction(direction), start(start), end(end), pos(pos)
{
}

int
TileLine::
len() const
{
  //assert(start >= 0 && end >= start);

  return end - start + 1;
}

bool
TileLine::
hasPosition(const TilePosition &p) const
{
  if (direction == Direction::HORIZONTAL)
    return (p.iy == pos && p.ix >= start && p.ix <= end);
  else
    return (p.ix == pos && p.iy >= start && p.iy <= end);
}

TilePosition
TileLine::
startPos() const
{
  if (direction == Direction::HORIZONTAL)
    return TilePosition(start, pos);
  else
    return TilePosition(pos, start);
}

bool
TileLine::
isValid(LineValid &lineValid) const
{
  lineValid.partial = false;
  lineValid.errMsg  = "";

  if (len() > 5) {
    lineValid.errMsg = "Line too long";
    return false;
  }

  //---

  if ((sum % 5) != 0) {
    if (len() == 5) {
      lineValid.errMsg = "Not a multiple of 5";
      return false;
    }

    lineValid.partial = true;
    lineValid.errMsg  = "Not a multiple of 5 (yet)";
  }

  return true;
}

void
TileLine::
print(std::ostream &os) const
{
  if (direction == Direction::HORIZONTAL)
    os << "H: (" << start << "," << pos << ") (" << end << ") = " << sum;
  else
    os << "V: (" << pos << "," << start << ") (" << end << ") = " << sum;
}

void
TileLine::
print(std::ostream &os, const QString &errMsg) const
{
  print(os);

  if (errMsg != "")
    os << " (" << errMsg.toStdString() << ")";
}

//------

void
BoardDetails::
reset()
{
  valid = false;
  score = 0;

  validPositions.clear();
}

void
BoardDetails::
addValidPosition(const TilePosition &pos)
{
  validPositions.insert(pos);
}

//------

MoveTree::
MoveTree()
{
}

MoveTree::
~MoveTree()
{
  for (auto &child : children)
    delete child;
}

const MoveTree *
MoveTree::
maxLeaf() const
{
  ScoreTree scoreTree;

  updateScoreTree(scoreTree);

  if (scoreTree.empty())
    return nullptr;

  //---

#if 0
  for (const auto &st : scoreTree) {
    std::cerr << "Score=" << -st.first << "\n";

    for (const auto &t : st.second) {
      Moves moves;

      t->hierMoves(moves);

      std::cerr << " ";

      for (const auto &move : moves) {
        std::cerr << " ";

        move.print(std::cerr);
      }

      std::cerr << "\n";
    }
  }
#endif

  //---

  const MoveTrees &moveTrees = scoreTree.begin()->second;

  int nt = moveTrees.size();
  //assert(nt > 0);

  auto minTree = moveTrees[0];

  if (nt == 1)
    return minTree;

  // best is min or max depth ?
  auto minDepth = minTree->depth();

  for (int i = 1; i < nt; ++i) {
    auto tree = moveTrees[i];

    auto depth = tree->depth();

    if (depth < minDepth) {
      minDepth = depth;
      minTree  = tree;
    }
  }

  return minTree;
}

void
MoveTree::
updateScoreTree(ScoreTree &scoreTree) const
{
  // add tree if valid and not partial (non multiple of 5)
  if (! partial) {
    //assert((score % 5) == 0);

    scoreTree[-score].push_back(this);
  }

  if (! children.empty()) {
    for (const auto &child : children)
      child->updateScoreTree(scoreTree);
  }
}

int
MoveTree::
depth() const
{
  if (! parent)
    return 1;

  return parent->depth() + 1;
}

int
MoveTree::
size() const
{
  int s = 1;

  for (const auto &child : children)
    s += child->size();

  return s;
}

void
MoveTree::
hierMoves(Moves &moves) const
{
  // get move parent stack
  moves.push_back(move);

  auto parent = this->parent;

  while (parent) {
    if (parent->move.isValid())
      moves.push_back(parent->move);

    parent = parent->parent;
  }

  // reverse as in wrong order
  int nm = moves.size();

  for (int i = 0; i < nm/2; ++i)
    std::swap(moves[i], moves[nm - i - 1]);
}

void
MoveTree::
printDepth(std::ostream &os, int depth) const
{
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

void
MoveTree::
print(std::ostream &os) const
{
  printDepth(os, 0);
}

//------

Turn::
Turn(App *quinto, int ind) :
 quinto_(quinto), ind_(ind)
{
}

int
Turn::
score() const
{
  auto score = 0;

  for (const auto &move : moves())
    score += quinto_->moveScore(move);

  return score;
}

//------

}
