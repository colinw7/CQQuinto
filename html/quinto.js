window.addEventListener("load", eventWindowLoaded, false);

var TileOwner = Object.freeze({ "NONE":0, "PLAYER1":1, "PLAYER2":2, "BOARD":3, "TILE_SET":4 });

var Direction = Object.freeze({ "NONE":0, "HORIZONTAL":1, "VERTICAL":2 });

var Side = Object.freeze({ "NONE":0, "LEFT":1, "RIGHT":2, "TOP":3, "BOTTOM":4 });

var PlayerType = Object.freeze({ "NONE":0, "HUMAN":1, "COMPUTER":2 });

var PlayMode = Object.freeze({ "HUMAN_HUMAN":0, "HUMAN_COMPUTER":1,
                               "COMPUTER_HUMAN":2, "COMPUTER_COMPUTER":3 });

var Align = Object.freeze({ "LEFT":0, "HCENTER":1, "RIGHT":2 });

//------

var quinto = new Quinto();

function eventWindowLoaded () {
  if (canvasSupport()) {
    quinto.init();
  }
}

function canvasSupport () {
  return true;
  //return Modernizr.canvas;
}

function assert(condition, message) {
  if (! condition) {
    throw message || "Assertion failed";
  }
}

//------

function Quinto () {
  this.nx = 18;
  this.ny = 12;

  this.border = 2;

  this.handSize = 5;

  this.playMode = PlayMode.HUMAN_COMPUTER;

  this.boardTileSize  = 1;
  this.playerTileSize = 1;

  //---

  this.boardX = 0;
  this.boardY = 0;

  //---

  this.turns = [];

  //---

  this.mouseDown = false;

  this.mouseX1 = 0;
  this.mouseY1 = 0;
  this.mouseX2 = 0;
  this.mouseY2 = 0;
}

Quinto.prototype.init = function() {
  this.tileSet = new TileSet();

  this.player1 = new Player(TileOwner.PLAYER1, "Player"  , PlayerType.HUMAN   );
  this.player2 = new Player(TileOwner.PLAYER2, "Computer", PlayerType.COMPUTER);

  this.currentPlayer = this.player1;

  this.board = new Board();

  this.turn = new Turn(0);

  //---

  this.dragTile = new Tile(-1, 0);

  //---

  this.connectWidgets();

  //---

  this.player1.drawTiles();
  this.player2.drawTiles();

  //---

  this.updateState();
};

Quinto.prototype.connectWidgets = function() {
  this.canvas = document.getElementById("canvas");
  this.gc     = this.canvas.getContext("2d");

  //---

  var backButton   = document.getElementById("backButton");
  var cancelButton = document.getElementById("cancelButton");
  var applyButton  = document.getElementById("applyButton");

  backButton  .addEventListener('click', this.backPressed, false);
  cancelButton.addEventListener('click', this.cancelPressed, false);
  applyButton .addEventListener('click', this.applyPressed, false);

  //---

  var newGameButton = document.getElementById("newGameButton");

  newGameButton.addEventListener('click', this.newGamePressed, false);

  var modeSelect = document.getElementById("modeSelect");

  modeSelect.value = 1;

  modeSelect.addEventListener("change", this.modeSelectChanged, false);

  //---

  this.canvas.addEventListener("mousedown", this.eventMouseDown, false);
  this.canvas.addEventListener("mousemove", this.eventMouseMove, false);
  this.canvas.addEventListener("mouseup"  , this.eventMouseUp  , false);
};

Quinto.prototype.setPlayMode = function(mode) {
  if (mode !== this.playMode) {
    this.playMode = mode;

    if      (mode === PlayMode.HUMAN_COMPUTER) {
      this.player1.name = "Player";
      this.player2.name = "Computer";

      this.player1.type = PlayerType.HUMAN;
      this.player2.type = PlayerType.COMPUTER;
    }
    else if (mode === PlayMode.COMPUTER_HUMAN) {
      this.player1.name = "Computer";
      this.player2.name = "Player";

      this.player1.type = PlayerType.COMPUTER;
      this.player2.type = PlayerType.HUMAN;
    }
    else if (mode === PlayMode.HUMAN_HUMAN) {
      this.player1.name = "Player1";
      this.player2.name = "Player2";

      this.player1.type = PlayerType.HUMAN;
      this.player2.type = PlayerType.HUMAN;
    }
    else if (mode === PlayMode.COMPUTER_COMPUTER) {
      this.player1.name = "Computer1";
      this.player2.name = "Computer2";

      this.player1.type = PlayerType.COMPUTER;
      this.player2.type = PlayerType.COMPUTER;
    }
    else {
      assert(false, "mode");
    }

    this.newGame();
  }
};

Quinto.prototype.updateState = function() {
  this.updateWidgets();

  this.drawScreen();

  //---

  //this.showBestMove();
};

Quinto.prototype.updateWidgets = function() {
  var validScore = this.isTurnValid();
  var canUndo    = (this.turn.moves.length > 0);

  var cancelButton = document.getElementById("cancelButton");
  var backButton   = document.getElementById("backButton");
  var applyButton  = document.getElementById("applyButton");

  cancelButton.disabled = ! canUndo;
  backButton  .disabled = ! canUndo;
  applyButton .disabled = ! validScore.valid;
};

//------

Quinto.prototype.cancelPressed = function() {
console.log("Quinto.prototype.cancelPressed");
  quinto.cancel();
};

Quinto.prototype.cancel = function() {
console.log("Quinto.prototype.cancel");
  var n = this.turn.moves.length;

  for (var i = n - 1; i >= 0; --i) {
    var move = turn.move(i);

    this.undoMove(move);
  }

  turn.clear();

  //---

  this.updateState();
};

//------

Quinto.prototype.backPressed = function() {
console.log("Quinto.prototype.backPressed");
  quinto.back();
};

Quinto.prototype.back = function(move) {
console.log("Quinto.prototype.back", move);
  var n = this.turn.moves.length;

  if (n > 0) {
    var move = this.turn.move(n - 1);

    this.undoMove(move);

    this.turn.popMove();
  }

  //---

  this.updateState();
};

//------

Quinto.prototype.applyPressed = function() {
console.log("Quinto.prototype.applyPressed");
  assert(quinto.currentPlayer.type === PlayerType.HUMAN, "bad player");

  quinto.apply(true);
};

Quinto.prototype.apply = function(next) {
console.log("Quinto.prototype.apply", next);
  // check valid
  var validScore = this.isTurnValid();
console.log(validScore);

  if (! validScore.valid)
    return;

  assert((validScore.score % 5) === 0, "bad score");

  // update score
  this.currentPlayer.addScore(validScore.score);

  // get new tiles
  this.currentPlayer.drawTiles();

  //---

  this.nextTurn();

  //---

  this.updateState();

  //---

  if (next)
    this.computerMove();
};

Quinto.prototype.computerMove = function()  {
  if (this.gameOver)
    return;

  while (this.currentPlayer.type === PlayerType.COMPUTER) {
console.log("Quinto.prototype.computerMove");
    var currentPlayer = this.currentPlayer();

    while (this.currentPlayer() === currentPlayer && currentPlayer().canMove()) {
      // auto play computers best move
      this.playComputerMove();

      // if other player can move computer is done
      if (this.currentPlayer().canMove())
        break;

      // skip other player
      this.nextTurn();

      assert(this.currentPlayer() === currentPlayer, "Bad player");
    }

    // if computer can't move then check if game over
    if (this.currentPlayer() === currentPlayer) {
      this.nextTurn();

      // if other player can't move then game over
      if (! this.currentPlayer().canMove()) {
        this.setGameOver(true);
        break;
      }
    }
  }
};

//------

Quinto.prototype.newGamePressed = function() {
console.log("Quinto.prototype.newGamePressed");
  quinto.newGame();
};

Quinto.prototype.newGame = function() {
console.log("Quinto.prototype.newGame");
  var handSize = this.handSize;

  // move all board and player tiles back to tile set
  for (var i = 0; i < handSize; ++i) {
    var tile1 = this.player1.takeTile(i, /*nocheck*/true);
    var tile2 = this.player2.takeTile(i, /*nocheck*/true);

    if (tile1 !== null) this.tileSet.ungetTile(tile1);
    if (tile2 !== null) this.tileSet.ungetTile(tile2);
  }

  var nx = this.nx;
  var ny = this.ny;

  for (var iy = 0; iy < ny; ++iy) {
    for (var ix = 0; ix < nx; ++ix) {
      var pos = new TilePosition(ix, iy);

      if (this.board.cellTile(pos) !== null) {
        var tile = this.board.takeCellTile(pos);

        if (tile !== null) this.tileSet.ungetTile(tile);
      }
    }
  }

  //---

  // reset current player and player scores
  this.currentPlayer = this.player1;

  this.player1.score = 0;
  this.player2.score = 0;

  this.player1.canMove = true;
  this.player2.canMove = true;

  //---

  // reset turns
  this.turns = [];

  this.turn = new Turn(0);

  //---

  // shuffle tiles
  this.tileSet.shuffle();

  //---

  // redraw player tiles
  this.player1.drawTiles();
  this.player2.drawTiles();

  //---

  this.gameOver = false;

  var newGameButton = document.getElementById("newGameButton");

  newGameButton.text = "New Game";

  //---

  this.updateState();

  //---

  this.computerMove();
};

//------

Quinto.prototype.modeSelectChanged = function(e) {
  var value = e.target.value;

  if      (value === "0") { quinto.setPlayMode(PlayMode.HUMAN_HUMAN      ); }
  else if (value === "1") { quinto.setPlayMode(PlayMode.HUMAN_COMPUTER   ); }
  else if (value === "2") { quinto.setPlayMode(PlayMode.COMPUTER_HUMAN   ); }
  else if (value === "3") { quinto.setPlayMode(PlayMode.COMPUTER_COMPUTER); }
  else                    { assert(false, "bad mode"); }
};

//------

Quinto.prototype.playComputerMove = function() {
console.log("Quinto.prototype.playComputerMove");
  assert(this.currentPlayer.type === PlayerType.COMPUTER, "bad player");

  this.board.playBestMove(false);

  //---

  quinto.updateState();
};

Quinto.prototype.nextTurn = function() {
console.log("Quinto.prototype.nextTurn");
  // next turn
  this.turns.push(this.turn);

  var ind = this.turn.ind + 1;

  this.turn = new Turn(ind);

  //---

  // switch to next player
  this.currentPlayer = (this.currentPlayer === this.player1 ? this.player2 : this.player1);

  //---

  this.board.detailsValid  = false;
  this.board.bestMoveValid = false;

  //---

  this.currentPlayer.canMove = this.canMove();
};

Quinto.prototype.setGameOver = function(b) {
console.log("Quinto.prototype.gameOver", b);
  this.gameOver = b;

  var newGameButton = document.getElementById("newGameButton");

  if (b)
    newGameButton.text = "Game Over";
  else
    newGameButton.text = "New Game";

  this.updateState();
};

Quinto.prototype.canMove = function() {
console.log("Quinto.prototype.canMove");
  if (this.currentPlayer.numTiles() === 0)
    return false;

  var details = this.board.boardDetails();

  if (! details.valid)
    return false;

  if (details.validPositions.size === 0)
    return false;

/*
  var bestMove = this.board.getBestMove();

  if (! bestMove.valid)
    return false;
*/

  return true;
};

Quinto.prototype.isTurnValid = function() {
console.log("Quinto.prototype.isTurnValid");
  var details = this.board.boardDetails();
console.log(details);

  return { "valid" : details.valid && ! details.partial, "score" : details.score };
};

Quinto.prototype.undoMove = function(move) {
console.log("Quinto.prototype.undoMove", move);
  this.doMoveParts(move.to, move.from);
};

Quinto.prototype.doMove = function(move) {
console.log("Quinto.prototype.doMove", move);
  this.doMoveParts(move.from, move.to);
};

Quinto.prototype.doMoveParts = function(from, to) {
console.log("Quinto.prototype.doMoveParts", from, to);
  if      (from.owner === TileOwner.PLAYER1) {
    assert(to.owner === TileOwner.BOARD, "Bad to owner");
console.log("from Player1 -> Board");

    var fromTile = this.player1.takeTile(from.pos.ix, /*nocheck*/false);
    assert(fromTile !== null, "null player from tile");

    fromTile.player = from.owner;
    fromTile.turn   = this.turn.ind;

    this.board.setCellTile(to.pos, fromTile);
  }
  else if (from.owner === TileOwner.PLAYER2) {
console.log("from Player2 -> Board");
    assert(to.owner === TileOwner.BOARD, "Bar to owner");

    var fromTile = this.player2.takeTile(from.pos.ix, /*nocheck*/false);
    assert(fromTile !== null, "null player from tile");

    fromTile.player = from.owner;
    fromTile.turn   = this.turn.ind;

    this.board.setCellTile(to.pos, fromTile);
  }
  else if (from.owner === TileOwner.BOARD) {
    if      (to.owner === TileOwner.PLAYER1 ||
             to.owner === TileOwner.PLAYER2) {
console.log("from Board -> Player1/2");
      var fromTile = this.board.takeCellTile(from.pos);
      assert(fromTile !== null, "null board from tile");

      if (to.owner === TileOwner.PLAYER1)
        this.player1.addTile(fromTile, to.pos.ix);
      else
        this.player2.addTile(fromTile, to.pos.ix);
    }
    else if (to.owner === TileOwner.BOARD) {
console.log("from Board -> Board");
      var fromTile = this.board.takeCellTile(from.pos);
      assert(fromTile !== null, "null board from tile");

      board.setCellTile(to.pos, fromTile);
    }
  }
  else {
    assert(false, "bad move");
  }
};

Quinto.prototype.moveScore = function(move) {
console.log("Quinto.prototype.moveScore", move);
  var to = move.to();

  if (to.owner === TileOwner.BOARD) {
    var tile = this.board.cellTile(to.pos);

    if (tile !== null)
      return tile.value;
  }

  return 0;
};

Quinto.prototype.addMove = function(move) {
console.log("Quinto.prototype.addMove", move);
  this.turn.addMove(move);
};

//------

Quinto.prototype.drawScreen = function() {
  var nx = this.nx;
  var ny = this.ny;

  //---

  this.gc.clearRect(0, 0, canvas.width, canvas.height);

  var w = this.canvas.width  - 2*this.border;
  var h = this.canvas.height - 4*this.border;

  var ny1 = ny + 2; // +2 for player tiles (top) and score (bottom)

  this.boardTileSize = Math.min(w/nx, h/ny1);

  this.boardX = (w - nx *this.boardTileSize)/2 + this.border;
  this.boardY = (h - ny1*this.boardTileSize)/2 + 2*this.border + this.boardTileSize;

  this.playerTileSize = this.boardTileSize*0.6;

  //---

  // draw player tiles
  if      (this.playMode === PlayMode.HUMAN_COMPUTER) {
    if (this.currentPlayer === this.player1 && this.player1.type === PlayerType.HUMAN)
      this.drawPlayerTiles(this.player1, this.canvas.width/2, Align.HCENTER);
  }
  else if (this.playMode === PlayMode.COMPUTER_HUMAN) {
    if (this.currentPlayer === this.player2 && this.player2.type === PlayerType.HUMAN)
      this.drawPlayerTiles(this.player2, this.canvas.width/2, Align.HCENTER);
  }
  else if (this.playMode === PlayMode.HUMAN_HUMAN) {
    this.drawPlayerTiles(this.player1, this.canvas.width/2, Align.RIGHT);
    this.drawPlayerTiles(this.player2, this.canvas.width/2, Align.LEFT );
  }
  else if (this.playMode === PlayMode.COMPUTER_COMPUTER) {
    this.drawPlayerTiles(this.player1, this.canvas.width/2, Align.RIGHT);
    this.drawPlayerTiles(this.player2, this.canvas.width/2, Align.LEFT );
  }
  else {
    assert(false, "bad mode");
  }

  //----

  var details = this.board.boardDetails();

  //----

  // draw board tiles
  var turnInd = quinto.turn.ind;

  for (var iy = 0; iy < ny; ++iy) {
    for (var ix = 0; ix < nx; ++ix) {
      var pos = new TilePosition(ix, iy);

      var tile = this.board.cellTile(pos);

      var bgColor = "#ffffff";
      var fgColor = "#000000";

      if (tile !== null) {
        bgColor = "#c0c1a0";

        var current = (tile.turn === turnInd);

        if      (current)
          fgColor = "#7459aa";
        else if (tile.owner === TileOwner.PLAYER1)
          fgColor = "#3d4c7f";
        else
          fgColor = "#366338";
      }

      this.drawTile(pos, bgColor, fgColor);
    }
  }

  //----

  // draw valid positions
  var bgColor = "#add4ab";
  var fgColor = "#000000";

  for (var position of details.validPositions.values()) {
    this.drawTile(position, bgColor, fgColor);
  }
};

Quinto.prototype.drawPlayerTiles = function(player, x, align) {
  var dx = 0;

  var w = player.drawWidth();

  var flip = false;

  if      (align === Align.HCENTER) {
    dx   = -w/2;
    flip = false;
  }
  else if (align === Align.RIGHT) {
    dx   = -w - 8;
    flip = false;
  }
  else {
    dx   = 8;
    flip = true;
  }

  player.draw(x + dx, 20, flip);
};

Quinto.prototype.drawTile = function(pos, bgColor, fgColor) {
  var tile = this.board.cellTile(pos);

  var x1 = this.boardX + pos.ix*this.boardTileSize;
  var y1 = this.boardY + pos.iy*this.boardTileSize;
  var x2 = x1 + this.boardTileSize;
  var y2 = y1 + this.boardTileSize;

  this.gc.fillStyle   = bgColor;
  this.gc.strokeStyle = fgColor;

  if (tile !== null) {
    tile.draw(x1, y1, this.boardTileSize, bgColor, fgColor);
  }
  else {
    this.gc.beginPath();

    this.gc.moveTo(x1, y1);
    this.gc.lineTo(x2, y1);
    this.gc.lineTo(x2, y2);
    this.gc.lineTo(x1, y2);

    this.gc.closePath();

    this.gc.fill();
    this.gc.stroke();
  }
};

Quinto.prototype.drawDrag = function() {
  if      (this.pressTileData.owner === TileOwner.PLAYER1) {
    var tile = this.player1.tiles[this.pressTileData.pos.ix];

    if (tile === null)
      return;

    this.dragTile.value = tile.value;
  }
  else if (this.pressTileData.owner === TileOwner.PLAYER2) {
    var tile = this.player2.tiles[this.pressTileData.pos.ix];

    if (tile === null)
      return;

    this.dragTile.value = tile.value;
  }
  else if (this.pressTileData.owner === TileOwner.BOARD) {
    var pos  = this.pressTileData.pos;
    var tile = this.board.cellTile(pos);

    if (tile === null)
      return;

    this.dragTile.value = tile.value;
  }
  else {
    return;
  }

  var bgColor = "#ffffff";
  var fgColor = "#000000";

  this.dragTile.draw(this.mouseX2, this.mouseY2, this.playerTileSize, bgColor, fgColor);
};

Quinto.prototype.posToTileData = function(x, y) {
  var tileData = new TileData(TileOwner.NONE, new TilePosition(0, 0));

  var nx = quinto.nx;
  var ny = quinto.ny;

  var ix = Math.floor((x - quinto.boardX)/quinto.boardTileSize);
  var iy = Math.floor((y - quinto.boardY)/quinto.boardTileSize);

  if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
    tileData.owner = TileOwner.BOARD;
    tileData.pos   = new TilePosition(ix, iy);

    return tileData;
  }

  //---

  var player1 = quinto.player1;
  var player2 = quinto.player2;

  var handSize = quinto.handSize;

  iy = Math.floor((y - player1.tileY)/quinto.playerTileSize);

  if (iy === 0) {
    if (this.playMode === PlayMode.HUMAN_COMPUTER ||
        this.playMode === PlayMode.HUMAN_HUMAN) {
      ix = Math.floor((x - player1.tileX)/quinto.playerTileSize);

      if (ix >= 0 && ix < handSize) {
        tileData.owner = TileOwner.PLAYER1;
        tileData.pos   = new TilePosition(ix, iy);

        return tileData;
      }
    }

    if (this.playMode === PlayMode.COMPUTER_HUMAN ||
        this.playMode === PlayMode.HUMAN_HUMAN) {
      ix = Math.floor((x - player2.tileX)/quinto.playerTileSize);

      if (ix >= 0 && ix < handSize) {
        tileData.owner = TileOwner.PLAYER2;
        tileData.pos   = new TilePosition(ix, iy);

        return tileData;
      }
    }
  }

  return tileData;
};

Quinto.prototype.eventMouseDown = function(e) {
console.log("Quinto.prototype.eventMouseDown", e);
  quinto.mouseDown = true;

  var rect = quinto.canvas.getBoundingClientRect();

  quinto.mouseX1 = e.clientX - rect.left;
  quinto.mouseY1 = e.clientY - rect.top;

  quinto.mouseX2 = quinto.mouseX1;
  quinto.mouseY2 = quinto.mouseY1;

  quinto.pressTileData = quinto.posToTileData(quinto.mouseX1, quinto.mouseY1);

  if     (quinto.pressTileData.owner === TileOwner.PLAYER1 ||
          quinto.pressTileData.owner === TileOwner.PLAYER2) {
    var currentPlayer = quinto.currentPlayer;

    if ((currentPlayer === quinto.player1 && quinto.pressTileData.owner !== TileOwner.PLAYER1) ||
        (currentPlayer === quinto.player2 && quinto.pressTileData.owner !== TileOwner.PLAYER2)) {
      quinto.pressTileData.owner = TileOwner.NONE;
    }
  }

  quinto.drawScreen();

  quinto.drawDrag();
};

Quinto.prototype.eventMouseMove = function(e) {
  if (quinto.mouseDown) {
    var rect = quinto.canvas.getBoundingClientRect();

    quinto.mouseX2 = e.clientX - rect.left;
    quinto.mouseY2 = e.clientY - rect.top;

    quinto.drawScreen();

    quinto.drawDrag();
  }
};

Quinto.prototype.eventMouseUp = function(e) {
console.log("Quinto.prototype.eventMouseUp", e);
  var rect = quinto.canvas.getBoundingClientRect();

  quinto.mouseX2 = e.clientX - rect.left;
  quinto.mouseY2 = e.clientY - rect.top;

  quinto.releaseTileData = quinto.posToTileData(quinto.mouseX2, quinto.mouseY2);

  quinto.mouseDown = false;

  quinto.drawScreen();

  //---

  // player -> board
  if      (quinto.pressTileData.owner === TileOwner.PLAYER1 ||
           quinto.pressTileData.owner === TileOwner.PLAYER2) {
    if (quinto.releaseTileData.owner !== TileOwner.BOARD)
      return;

    // ensure destination is empty (TODO: only to valid square)
    var tile = quinto.board.cellTile(quinto.releaseTileData.pos);

    if (tile !== null)
      return;
  }
  // board -> player or board
  else if (quinto.pressTileData.owner === TileOwner.BOARD) {
    // board -> player1
    if      (quinto.releaseTileData.owner === TileOwner.PLAYER1) {
      if (quinto.player1 !== quinto.currentPlayer)
        return;
    }
    // board -> player2
    else if (quinto.releaseTileData.owner === TileOwner.PLAYER2) {
      if (quinto.player2 !== quinto.currentPlayer)
        return;
    }
    // board -> board
    else if (quinto.releaseTileData.owner === TileOwner.BOARD) {
      var pressTile = quinto.board.cellTile(quinto.pressTileData.pos);

      if (pressTile === null)
        return;

      var releaseTile = quinto.board.cellTile(quinto.releaseTileData.pos);

      if (releaseTile !== null)
        return;
    }
    else
      return;
  }
  else {
    return;
  }

  //---

  var move = new Move(quinto.pressTileData, quinto.releaseTileData);

  quinto.addMove(move);

  quinto.doMove(move);

  quinto.updateState();
};

//------

function TileData (owner, pos) {
  this.owner = owner;
  this.pos   = pos;
}

//------

function TileSet () {
  this.tiles = [];

  this.addTiles( 7, 0);
  this.addTiles( 6, 1);
  this.addTiles( 6, 2);
  this.addTiles( 7, 3);
  this.addTiles(10, 4);
  this.addTiles( 6, 5);
  this.addTiles(10, 6);
  this.addTiles(14, 7);
  this.addTiles(12, 8);
  this.addTiles(12, 9);

  this.shuffle();
}

TileSet.prototype.addTile = function(value) {
  var ind = this.tiles.length;

  var tile = new Tile(ind, value);

  this.tiles.push(tile);
};

TileSet.prototype.addTiles = function(n, value) {
  for (var i = 0; i < n; ++i) {
    this.addTile(value);
  }
};

TileSet.prototype.shuffle = function() {
  function getRandomInt(max) {
    return Math.floor(Math.random() * Math.floor(max));
  }

  var nt = this.tiles.length;

  var ns = 100;

  for (var i = 0; i < ns; ++i) {
    var pos1 = getRandomInt(nt);
    var pos2 = getRandomInt(nt);

    while (pos1 === pos2)
      pos2 = getRandomInt(nt);

    var t            = this.tiles[pos1];
    this.tiles[pos1] = this.tiles[pos2];
    this.tiles[pos2] = t;
  }
};

TileSet.prototype.getTile = function() {
  var tile = this.tiles.pop();

  tile.owner  = TileOwner.NONE;
  tile.player = TileOwner.NONE;

  return tile;
};

TileSet.prototype.ungetTile = function(tile) {
  tile.owner  = TileOwner.TILE_SET;
  tile.player = TileOwner.NONE;

  this.tiles.push(tile);
};

//------

function Tile (ind, value) {
  this.ind   = ind;
  this.value = value;
}

Tile.prototype.draw = function(x, y, s, bgColor, fgColor) {
  var x1 = x;
  var y1 = y;
  var x2 = x1 + s;
  var y2 = y1 + s;

  quinto.gc.beginPath();

  quinto.gc.moveTo(x1, y1);
  quinto.gc.lineTo(x2, y1);
  quinto.gc.lineTo(x2, y2);
  quinto.gc.lineTo(x1, y2);

  quinto.gc.closePath();

  quinto.gc.strokeStyle = "#000000";
  quinto.gc.fillStyle   = bgColor;

  quinto.gc.fill();
  quinto.gc.stroke();

  quinto.gc.fillStyle = fgColor;

  var text = String(this.value);

  quinto.gc.font = String(s) + "px Arial";

  var ts = quinto.gc.measureText(text);

  var dx = (s - ts.width)/2;
  var dy = s*0.15;

  quinto.gc.fillText(text, x1 + dx, y2 - dy);
};

//------

function Player (owner, name, type) {
  this.owner = owner;
  this.name  = name;
  this.type  = type;

  this.tiles = [];

  var handSize = quinto.handSize;

  for (var i = 0; i < handSize; ++i)
    this.tiles.push(null);

  this.score = 0;

  this.canMove = true;

  this.tileX = 0;
  this.tileY = 0;
}

Player.prototype.numTiles = function() {
  var handSize = quinto.handSize;

  var n = 0;

  for (var i = 0; i < handSize; ++i)
    if (this.tiles[i] !== null)
      ++n;

  return n;
};

Player.prototype.drawTiles = function() {
  var nt = this.tiles.length;

  for (var i = 0; i < nt; ++i) {
    if (this.tiles[i] !== null)
      continue;

    var tile = quinto.tileSet.getTile();

    if (tile === null)
      break;

    tile.owner  = this.owner;
    tile.player = TileOwner.NONE;

    this.tiles[i] = tile;
  }
};

Player.prototype.takeTile = function(i, nocheck) {
console.log("Player.prototype.takeTile", i, nocheck);
  assert(i >= 0 && i < quinto.handSize, "Out of range");

  var tile = this.tiles[i];

  if (nocheck) {
    if (tile === null)
      return null;
  }
  else {
    assert(tile !== null, "null tile");
  }

  tile.owner  = TileOwner.NONE;
  tile.player = TileOwner.NONE;

  this.tiles[i] = null;

  return tile;
};

Player.prototype.addTile = function(tile, i) {
  assert(i >= 0 && i < quinto.handSize, "Out of range");

  assert(this.tiles[i] === null, "add at non-null");

  this.tiles[i] = tile;

  tile.owner  = this.owner;
  tile.player = TileOwner.NONE;
};

Player.prototype.addScore = function(score) {
  this.score += score;
};

Player.prototype.draw = function(x, y, flip) {
  var nt = this.tiles.length;

  this.tileY = y;

  var x1 = x;
  var y1 = y;

  var bgColor = "#ffffff";
  var fgColor = "#000000";

  var s = quinto.playerTileSize;

  quinto.gc.font = String(s) + "px Arial";

  var ts1 = quinto.gc.measureText(this.name);

  var scoreText = String(this.score);

  var ts2 = quinto.gc.measureText(scoreText);

  if (! flip) {
    if (quinto.currentPlayer === this)
      quinto.gc.fillStyle = "#64d444";
    else
      quinto.gc.fillStyle = "#000000";

    quinto.gc.fillText(this.name, x1, y1 + s - 4);
  
    x1 += ts1.width + 8;

    //---

    quinto.gc.fillStyle = "#664444";

    quinto.gc.fillText(scoreText, x1, y1 + s - 4);

    x1 += ts2.width + 40;

    //---

    this.tileX = x1;

    for (var i = 0; i < nt; ++i) {
      if (this.tiles[i] !== null) {
        this.tiles[i].draw(x1, y1, s, bgColor, fgColor);
      }

      x1 += quinto.playerTileSize;
    }
  }
  else {
    this.tileX = x1;

    for (var i = 0; i < nt; ++i) {
      if (this.tiles[i] !== null) {
        this.tiles[i].draw(x1, y1, s, bgColor, fgColor);
      }

      x1 += quinto.playerTileSize;
    }

    //---

    x1 += 40;

    if (quinto.currentPlayer === this)
      quinto.gc.fillStyle = "#64d444";
    else
      quinto.gc.fillStyle = "#000000";

    quinto.gc.fillText(this.name, x1, y1 + s - 4);

    x1 += ts1.width + 8;

    //---

    quinto.gc.fillStyle = "#664444";

    quinto.gc.fillText(scoreText, x1, y1 + s - 4);
  }
};

Player.prototype.drawWidth = function() {
  var nt = this.tiles.length;

  var s = quinto.playerTileSize;

  quinto.gc.font = String(s) + "px Arial";

  var ts1 = quinto.gc.measureText(this.name);

  var scoreText = String(this.score);

  var ts2 = quinto.gc.measureText(scoreText);

  return quinto.playerTileSize*nt + ts1.width + 40 + ts2.width + 8;
};

//------

function Board () {
  var nx = quinto.nx;
  var ny = quinto.ny;

  this.tiles = [];

  for (var iy = 0; iy < ny; ++iy)
    this.tiles.push([]);

  for (var iy = 0; iy < ny; ++iy) {
    for (var ix = 0; ix < nx; ++ix) {
      this.tiles[iy].push(null);
    }
  }

  this.details      = new BoardDetails();
  this.detailsValid = false;

  this.bestMove      = new BestMove();
  this.bestMoveValid = false;
}

Board.prototype.cellTile = function(pos) {
  if (pos.ix >= 0 && pos.ix < quinto.nx && pos.iy >= 0 && pos.iy < quinto.ny) {
    return this.tiles[pos.iy][pos.ix];
  }

  return null;
};

Board.prototype.takeCellTile = function(pos) {
  assert(pos.ix >= 0 && pos.ix < quinto.nx && pos.iy >= 0 && pos.iy < quinto.ny, "bad pos");

  var tile = this.tiles[pos.iy][pos.ix];
  assert(tile !== null, "bad tile");

  this.tiles[pos.iy][pos.ix] = null;

  tile.owner  = TileOwner.NONE;
  tile.player = TileOwner.NONE;

  this.detailsValid  = false;
  this.bestMoveValid = false;

  return tile;
};

Board.prototype.setCellTile = function(pos, tile) {
  assert(pos.ix >= 0 && pos.ix < quinto.nx && pos.iy >= 0 && pos.iy < quinto.ny, "Bad Pos");

  assert(this.tiles[pos.iy][pos.ix] === null, "null tile");

  tile.owner = TileOwner.BOARD;

  this.tiles[pos.iy][pos.ix] = tile;

  this.detailsValid  = false;
  this.bestMoveValid = false;
};

Board.prototype.playBestMove = function() {
console.log("Board.prototype.playBestMove");
  var bestMove = this.getBestMove();

  if (! bestMove.valid) {
console.log("No best move");
    return;
  }

console.log(bestMove);
  var nm = bestMove.moves.length;

  for (var i = 0; i < nm; ++i) {
    quinto.doMove(bestMove.moves[i]);
  }

  quinto.apply();
};

Board.prototype.showBestMove = function() {
console.log("Board.prototype.showBestMove");
};

Board.prototype.getBestMove = function() {
console.log("Board.prototype.getBestMove");
  if (! this.bestMoveValid) {
    this.calcBestMove();

    this.bestMoveValid = true;
  }

  return this.bestMove;
};

Board.prototype.calcBestMove = function() {
  this.bestMove.reset();

  var moveTree = this.boardMoveTree();

  if (moveTree === null) {
conole.log("no tree");
    return { "valid": false, "moves": [], "score": 0 };
  }

console.log(moveTree);
  var maxLeaf = moveTree.maxLeaf();
console.log(maxLeaf);

  if (maxLeaf !== null) {
    maxLeaf.hierMoves(bestMove.moves);

    bestMove.score = maxLeaf.score;
  }
};

Board.prototype.boardMoveTree = function() {
  return this.buildBoardMoveTree(0);
};

Board.prototype.buildBoardMoveTree = function(depth) {
console.log("Board.prototype.buildBoardMoveTree", depth);
  assert(depth <= 5, "bad depth");

  var moves = this.boardMoves();

  if (! moves.valid) {
console.log("no moves");
    return null;
  }

console.log(moves.moves);

  var tree = new MoveTree();

  tree.partial = moves.partial;
  tree.score   = moves.score;

  for (var i = 0; i < moves.moves.length; ++i) {
    var move = moves.moves[i];

    quinto.doMoveParts(move.from, move.to);

    var childTree = this.buildBoardMoveTree(depth + 1);

    if (childTree !== null) {
      childTree.parent = tree;
      childTree.move   = move;

      tree.children.push(childTree);
    }

    quinto.doMoveParts(move.to, move.from);
  }

  return tree;
};

Board.prototype.boardMoves = function() {
console.log("Board.prototype.boardMoves");
  var details = this.boardDetails();

  if (! details.valid) {
    return { "valid": false, "score": 0, "moves": [], "partial": false };
  }

  //---

  var moves   = [];
  var score   = details.score;
  var partial = details.partial;

  //---

  var handSize = quinto.handSize;

  var currentPlayer = quinto.currentPlayer;
console.log(currentPlayer);

  var playerOwner = currentPlayer.owner;
console.log(playerOwner);

  for (var position of details.validPositions.values()) {
    var values = new Set();

    for (var i = 0; i < handSize; ++i) {
      var tile = currentPlayer.tiles[i];
      if (tile === null) continue;

      if (values.has(tile.value))
        continue;

      var pos = new TilePosition(i, 0);

      var from = new TileData(playerOwner, pos);
      var to   = new TileData(TileOwner.BOARD, position);

      var move = new Move(from, to);

      moves.push(move);

      values.add(tile.value);
    }
  }
console.log(moves);

  return { "valid": true, "score": score, "moves": moves, "partial": partial };
};

Board.prototype.boardDetails = function() {
console.log("Board.prototype.boardDetails");
  if (! this.detailsValid) {
    this.calcBoardDetails();

    this.detailsValid = true;
  }

  return this.details;
};

// depends on cells, current turn
Board.prototype.calcBoardDetails = function() {
  this.details.reset();

  this.details.valid   = true;
  this.details.partial = false;

  //---

  var turnInd = quinto.turn.ind;

  var nx = quinto.nx;
  var ny = quinto.ny;

  //---

  // count number of current move tiles in each row (y) and column (x)
  // save row/columns containing current move tiles
  var boardLines = new BoardLines();

  this.details.nt  = 0;
  this.details.npt = 0;

  for (var iy = 0; iy < ny; ++iy) {
    for (var ix = 0; ix < nx; ++ix) {
      var pos = new TilePosition(ix, iy);

      var tile = this.cellTile(pos);
      if (tile === null) continue;

      ++this.details.nt;

      if (tile.turn === turnInd) {
        boardLines.xinds.add(ix);
        boardLines.yinds.add(iy);

        ++this.details.npt;
      }
    }
  }

  //---

  // if board empty then must be valid
  // return center positions for valid positions
  if (this.details.nt === 0) {
    // single center position
    var ix1 = Math.floor((nx - 1)/2);
    var iy1 = Math.floor((ny - 1)/2);

    this.details.addValidPosition(new TilePosition(ix1, iy1));

    this.details.partial = true;

    return;
  }

  //---

  // no tiles placed yet (for current player) then must be valid,
  if (this.details.npt === 0) {
    assert(boardLines.xinds.size === 0 && boardLines.yinds.size === 0, "Tile count mismatch");

    // play off existing pieces (board not empty)
    for (var iy = 0; iy < ny; ++iy) {
      for (var ix = 0; ix < nx; ++ix) {
        var pos = new TilePosition(ix, iy);

        var tile = this.cellTile(pos);
        if (tile !== null) continue;

        // check if empty cell has any surrounding tiles
        var l_pos = pos.left  ();
        var r_pos = pos.right ();
        var t_pos = pos.top   ();
        var b_pos = pos.bottom();

        var l_tile = (ix > 0      ? this.cellTile(l_pos) : null);
        var r_tile = (ix < nx - 1 ? this.cellTile(r_pos) : null);
        var t_tile = (iy > 0      ? this.cellTile(t_pos) : null);
        var b_tile = (iy < ny - 1 ? this.cellTile(b_pos) : null);

        if (l_tile === null && r_tile === null && t_tile === null && b_tile === null )
          continue;

        //---

        // count run to left, right, top, bottom
        var l_count = (l_tile !== null ? this.countTiles(l_pos, Side.LEFT  ) : 0);
        var r_count = (r_tile !== null ? this.countTiles(r_pos, Side.RIGHT ) : 0);

        if (l_count + r_count + 1 > 5)
          continue;

        var t_count = (t_tile !== null ? this.countTiles(t_pos, Side.TOP   ) : 0);
        var b_count = (b_tile !== null ? this.countTiles(b_pos, Side.BOTTOM) : 0);

        if (t_count + b_count + 1 > 5)
          continue;

        //---

        this.details.addValidPosition(pos);
      }
    }

    // can't apply yet
    this.details.partial = true;

    return;
  }

  //---

  // get connected lines, length 2 or more, including at least one turn piece
  this.getBoardLines(boardLines);

  //---

  // check all lines
  for (var i = 0; i < boardLines.hlines.length; ++i) {
    var line = boardLines.hlines[i];

    var values = line.isValid();

    this.details.valid   = values.valid;
    this.details.partial = values.partial;
    this.details.errMsg  = values.errMsg;

    //console.log(details.errMsg);

    if (! this.details.valid)
      return;
  }

  for (var i = 0; i < boardLines.vlines.length; ++i) {
    var line = boardLines.vlines[i];

    var values = line.isValid();

    this.details.valid   = values.valid;
    this.details.partial = values.partial;
    this.details.errMsg  = values.errMsg;

    //console.log(details.errMsg);

    if (! this.details.valid)
      return;
  }

  //---

  // single piece played then check row or column
  if (this.details.npt === 1) {
    assert(boardLines.xinds.size === 1 && boardLines.yinds.size === 1, "Tile count mismatch");

    var ix1 = boardLines.xinds.values().next().value;
    var iy1 = boardLines.yinds.values().next().value;

    //---

    // play off vertical lines of existing piece (board not empty)
    for (var iy = 0; iy < ny; ++iy) {
      var pos = new TilePosition(ix1, iy);

      var tile = this.cellTile(pos);
      if (tile !== null) continue;

      // check if empty cell has any surrounding tiles
      var t_pos = pos.top   ();
      var b_pos = pos.bottom();

      var t_tile = (iy > 0      ? this.cellTile(t_pos) : null);
      var b_tile = (iy < ny - 1 ? this.cellTile(b_pos) : null);

      if (t_tile === null && b_tile === null)
        continue;

      //---

      // count runs at top and bottom
      var t_count = (t_tile !== null ? this.countTiles(t_pos, Side.TOP   ) : 0);
      var b_count = (b_tile !== null ? this.countTiles(b_pos, Side.BOTTOM) : 0);

      if (t_count + b_count + 1 > 5)
        continue;

      //---

      this.details.addValidPosition(pos);
    }

    //---

    // play off horizontal lines of existing piece (board not empty)
    for (var ix = 0; ix < nx; ++ix) {
      var pos = new TilePosition(ix, iy1);

      var tile = this.cellTile(pos);
      if (tile !== null) continue;

      // check if empty cell has any surrounding tiles
      var l_pos = pos.left ();
      var r_pos = pos.right();

      var l_tile = (ix > 0      ? this.cellTile(l_pos) : null);
      var r_tile = (ix < nx - 1 ? this.cellTile(r_pos) : null);

      if (l_tile === null && r_tile === null)
        continue;

      //---

      // count run to left, right, top, bottom
      var l_count = this.countTiles(l_pos, Side.LEFT  );
      var r_count = this.countTiles(r_pos, Side.RIGHT );

      if (l_count + r_count + 1 > 5)
        continue;

      //---

      this.details.addValidPosition(pos);
    }

    //---

    var pos = new TilePosition(ix1, iy1);

    var tile = this.cellTile(pos);
    assert(tile !== null, "bad tile");

    this.details.score   = tile.value;
    this.details.partial = ((this.details.score % 5) !== 0);

    return;
  }

  //---

  // two or more pieces. must be in a single row or column
  if (boardLines.xinds.size > 1 && boardLines.yinds.size > 1) {
    this.details.valid  = false;
    this.details.errMsg = "Disjoint pieces";

    return;
  }

  //---

  var horizontal = (boardLines.xinds.size > 1);

  //---

  var lines = [];

  if (horizontal)
    lines = boardLines.hlines;
  else
    lines = boardLines.vlines;

  assert(lines.length !== 0, "Lines empty");

  //---

  // score all lines
  this.details.score = 0;

  for (var i = 0; i < boardLines.hlines.length; ++i)
    this.details.score += boardLines.hlines[i].sum;

  for (var i = 0; i < boardLines.vlines.length; ++i)
    this.details.score += boardLines.vlines[i].sum;

  //---

  this.details.partial = ((this.details.score % 5) !== 0);

  //---

  // add valid positions (end of lines)
  for (var i = 0; i < lines.length; ++i) {
    var line = lines[i];

    if (line.len() >= 5)
      continue;

    if (horizontal) {
      var ix1 = line.start - 1;
      var ix2 = line.end   + 1;

      if (ix1 >= 0 ) this.details.addValidPosition(new TilePosition(ix1, line.pos));
      if (ix2 <  nx) this.details.addValidPosition(new TilePosition(ix2, line.pos));
    }
    else {
      var iy1 = line.start - 1;
      var iy2 = line.end   + 1;

      if (iy1 >= 0 ) this.details.addValidPosition(new TilePosition(line.pos, iy1));
      if (iy2 <  ny) this.details.addValidPosition(new TilePosition(line.pos, iy2));
    }
  }

  //---

  if (! this.details.partial) {
    assert((this.details.score % 5) === 0, "bad score");
  }
};

Board.prototype.getBoardLines = function(boardLines) {
console.log("Board.prototype.getBoardLines", boardLines);
  var shline = new TileLine(Direction.NONE, -1, -1, -1);
  var svline = new TileLine(Direction.NONE, -1, -1, -1);

  var turnInd = quinto.turn.ind;

  var nx = quinto.nx;
  var ny = quinto.ny;

  // get horizontal sequence
  for (var iy of boardLines.yinds.values()) {
    var ix = 0;

    while (ix < nx) {
      // find first tile
      while (ix < nx && ! this.cellTile(new TilePosition(ix, iy)))
        ++ix;

      if (ix >= nx)
        break;

      var ixs = ix;

      // find last tile
      while (ix < nx && this.cellTile(new TilePosition(ix, iy)))
        ++ix;

      var ixe = ix - 1;

      var line = new TileLine(Direction.HORIZONTAL, ixs, ixe, iy);

      //---

      // ignore unit line
      if (line.len() === 1) {
        shline = line;
        continue;
      }

      //---

      // line must have current tile in it
      line.sum = 0;

      var current = 0;

      for (var ix = line.start; ix <= line.end; ++ix) {
        var tile = this.cellTile(new TilePosition(ix, iy));

        if (tile.turn === turnInd)
          ++current;

        line.sum += tile.value;
      }

      if (! current)
        continue;

      line.current = current;

      //---

      boardLines.hlines.push(line);
    }
  }
console.log(boardLines.hlines);

  // get vertical sequences
  for (var ix of boardLines.xinds.values()) {
    var iy = 0;

    while (iy < ny) {
      // find first tile
      while (iy < ny && ! this.cellTile(new TilePosition(ix, iy)))
        ++iy;

      if (iy >= ny)
        break;

      var iys = iy;

      // find last tile
      while (iy < ny && this.cellTile(new TilePosition(ix, iy)))
        ++iy;

      var iye = iy - 1;

      var line = new TileLine(Direction.VERTICAL, iys, iye, ix);

      //---

      // ignore unit line
      if (line.len() === 1) {
        svline = line;
        continue;
      }

      //---

      // line must have current tile in it
      line.sum = 0;

      var current = 0;

      for (var iy = line.start; iy <= line.end; ++iy) {
        var tile = this.cellTile(new TilePosition(ix, iy));

        if (tile.turn === turnInd)
          ++current;

        line.sum += tile.value;
      }

      if (! current)
        continue;

      line.current = current;

      //---

      boardLines.vlines.push(line);
    }
  }
console.log(boardLines.vlines);

  if (boardLines.hlines.length === 0 && boardLines.vlines.length === 0) {
    var tile = this.cellTile(new TilePosition(shline.start, shline.pos));

    shline.sum = tile.value;
    svline.sum = tile.value;

    boardLines.hlines.push(shline);
    boardLines.vlines.push(svline);
  }
};

Board.prototype.countTiles = function(pos, side) {
  //assert(this.cellTile(pos) !== null, "null tile");

  var nx = quinto.nx;
  var ny = quinto.ny;

  // count run to left, right, top or bottom
  var count = 1;

  var ix = pos.ix;
  var iy = pos.iy;

  if      (side === Side.LEFT) {
    --ix; while (ix >=      0 && this.cellTile(new TilePosition(ix, iy))) { ++count; --ix; }
  }
  else if (side === Side.RIGHT) {
    ++ix; while (ix <= nx - 1 && this.cellTile(new TilePosition(ix, iy))) { ++count; ++ix; }
  }
  else if (side === Side.TOP) {
    --iy; while (iy >=      0 && this.cellTile(new TilePosition(ix, iy))) { ++count; --iy; }
  }
  else if (side === Side.BOTTOM) {
    ++iy; while (iy <= ny - 1 && this.cellTile(new TilePosition(ix, iy))) { ++count; ++iy; }
  }

  return count;
};

//------

function TileLine (direction, start, end, pos) {
  this.direction = direction;
  this.start     = start;
  this.end       = end;
  this.pos       = pos;
  this.sum       = 0;
  this.current   = 0;
}

TileLine.prototype.len = function() {
  return this.end - this.start + 1;
};

TileLine.prototype.hasPosition = function(p) {
  if (this.direction === Direction.HORIZONTAL)
    return (p.iy === this.pos && p.ix >= this.start && p.ix <= this.end);
  else
    return (p.ix === this.pos && p.iy >= this.start && p.iy <= this.end);
};

TileLine.prototype.startPos = function(p) {
  if (direction === Direction.HORIZONTAL)
    return new TilePosition(this.start, this.pos);
  else
    return new TilePosition(this.pos, this.start);
};

TileLine.prototype.isValid = function() {
  var valid   = true;
  var partial = false;
  var errMsg  = "";

  if (this.len() > 5) {
    return { "valid": false, "partial": partial, "errMsg": "Line too long" };
  }

  //---

  if ((this.sum % 5) !== 0) {
    if (this.len() === 5) {
      return { "valid": false, "partial": partial, "errMsg": "Not a multiple of 5" };
    }

    this.partial = true;
    this.errMsg  = "Not a multiple of 5 (yet)";
  }

  return { "valid": true, "partial": partial, "errMsg": "Not a multiple of 5" };
};

//------

function BoardDetails () {
  this.valid          = false;
  this.partial        = false;
  this.score          = 0;
  this.validPositions = new Set();
  this.errMsg         = "";
}

BoardDetails.prototype.reset = function() {
  this.valid          = false;
  this.partial        = false;
  this.score          = 0;
  this.validPositions = new Set();
  this.errMsg         = "";
};

BoardDetails.prototype.addValidPosition = function(pos) {
//console.log("Tile.prototype.addValidPosition", pos);

  this.validPositions.add(pos);
};

//------

function MoveTree () {
  this.parent    = null;
  this.move      = null;
  this.children  = [];
  this.partial   = false;
  this.score     = 0;
  this.scoreTree = new Map();
}

MoveTree.prototype.maxLeaf = function() {
console.log("MoveTree.prototype.maxLeaf");
  this.scoreTree = new Map();

  this.updateScoreTree(this.scoreTree);

  if (this.scoreTree.size === 0) {
console.log("no score tree");
    return null;
  }

console.log(this.scoreTree);

  //---

  var moveTrees = this.scoreTree.values().next().value;

  assert(moveTrees.length !== 0, "Empty Move Trees");

  var nt = moveTrees.length;

  var minTree = moveTrees[0];

  if (nt === 1)
    return minTree;

  var minDepth = minTree.depth;

  for (var i = 1; i < nt; ++i) {
    var tree = moveTrees[i];

    var depth = tree.depth();

    if (depth < minDepth) {
      minDepth = depth;
      minTree  = tree;
    }
  }

  return minTree;
};

MoveTree.prototype.updateScoreTree = function(scoreTree) {
console.log("MoveTree.prototype.updateScoreTree", scoreTree);
  if (! this.partial) {
    assert((this.score % 5) === 0, "bad score");

    scoreTree[-this.score].push_back(this);
  }

  if (! this.children.length) {
    for (var i = 0; i < this.children.length; ++i)
      child.updateScoreTree(scoreTree);
  }
};

MoveTree.prototype.depth = function() {
  if (this.parent === null)
    return 1;

  return this.parent.depth() + 1;
};

MoveTree.prototype.hierMoves = function() {
  this.moves.push(move);

  var parent = this.parent;

  while (parent !== null) {
    if (parent.move.isValid())
      this.moves.push(parent.move);

    parent = parent.parent;
  }

  // reverse as in wrong order
  var nm = moves.length;

  for (var i = 0; i < nm/2; ++i) {
    var t                  = this.moves[i];
    this.moves[i         ] = this.moves[nm - i - 1];
    this.moves[nm - i - 1] = t;
  }
};

//------

function Turn (ind) {
  this.ind   = ind;
  this.moves = [];
}

Turn.prototype.move = function(i) {
  return this.moves[i];
};

Turn.prototype.addMove = function(move) {
  this.moves.push(move);
};

Turn.prototype.popMove = function() {
  this.moves.pop();
};

Turn.prototype.score = function() {
  var score = 0;

  for (var i = 0; i < this.moves.length; ++i) {
    score += quinto.moveScore(moves[i]);
  }

  return score;
};

//------

function BestMove (from, to) {
  this.moves = [];
  this.score = 0;
}

BestMove.prototype.isValid = function() {
  return (this.moves.length > 0);
};

BestMove.prototype.reset = function() {
  this.moves = [];
  this.score = 0;
};

//------

function BoardLines () {
  this.xinds = new Set();
  this.yinds = new Set();

  this.hlines = [];
  this.vlines = [];
}

//------

function Move (from, to) {
  this.from = from;
  this.to   = to;
}

Move.prototype.isValid = function(pos) {
  return (this.from.owner !== TileOwner.NONE && this.to.owner !== TileOwner.NONE);
};

//------

function TilePosition (ix, iy) {
  this.ix = ix;
  this.iy = iy;
}

TilePosition.prototype.left = function() {
  return new TilePosition(this.ix - 1, this.iy);
};

TilePosition.prototype.right = function() {
  return new TilePosition(this.ix + 1, this.iy);
};

TilePosition.prototype.top = function() {
  return new TilePosition(this.ix, this.iy - 1);
};

TilePosition.prototype.bottom = function() {
  return new TilePosition(this.ix, this.iy + 1);
};
