# -*- coding: utf-8 -*-
import random
import sys

def isOnBoard(x, y):
    return (x>=0 and x<=7 and y>=0 and y<=7)

def isOnCorner(x, y):
    return (x==0 or x==7) and (y==0 or y==7)

def isValidMove(board, tile, xstart, ystart):
    if (board[xstart][ystart] != ' ') or (not isOnBoard(xstart, ystart)):
        return False
    board[xstart][ystart] = tile
    otherTile = 'O'
    if tile=='O':
        otherTile = 'X'

    tilesToFlip = []
    direction = [[0,1],[1,1],[1,0],[1,-1],[0,-1],[-1,-1],[-1,0],[-1,1]]
    for xd, yd in direction:
        x, y = xstart, ystart
        x += xd
        y += yd
        if isOnBoard(x, y) and board[x][y]==otherTile:
           x += xd
           y += yd
           if not isOnBoard(x, y):
               continue
           while board[x][y] == otherTile:
               x += xd
               y += yd
               if not isOnBoard(x, y):
                  break
           if not isOnBoard(x, y):
               continue
           if board[x][y] == tile:
               while True:
                   x -= xd
                   y -= yd
                   if x==xstart and y==ystart:
                       break
                   tilesToFlip.append([x, y])
    board[xstart][ystart] = ' ' # back to before
    if len(tilesToFlip)==0:
        return False

    return tilesToFlip

def getBoardCopy(cboard):
    dupeBoard = []
    for i in range(8):
        dupeBoard.append([" "]*8)
    for x in range(8):
        for y in range(8):
            dupeBoard[x][y] = cboard.board[x][y]
    return dupeBoard

class Player(object):
    def __init__(self):
        self.score = 0;                             #棋子个数
        self.playType = ''                          #棋子形状
        self.move = []                              #下次走的位置
        self.validMoves = []                        #可走的位置
        self._is_contion = True                     #是否可以继续

    def chooseType(self, computer):

        while not ('X' == self.playType or 'O' == self.playType):
            print("Do you want to be X or O?")
            self.playType = raw_input().upper()
            if 'X' == self.playType:
                computer.playType = 'O'
            else:
                computer.playType = 'X'

    def getScoreOfBoard(self, board):
        self.score = 0
        for x in range(8):
            for y in range(8):
                if board[x][y] == self.playType:
                    self.score += 1

    def getPlayerMove(self):
        DIGIT = '1 2 3 4 5 6 7 8'.split()
        while True:
            self.move = []
            print "Enter your move, or type quit to end the game."
            self.move = raw_input()
            if self.move == 'quit':
                return

            if len(self.move)==2 and (self.move[0] in DIGIT) and (self.move[1] in DIGIT):
                x = int(self.move[0]) - 1
                y = int(self.move[1]) - 1
                if [x, y] in (self.validMoves):
                    return
                else:
                    continue
            else:
                print "That is not a valid move. Type the x digit (1-8), then the y digit (1-8)."
                print "For example, 81 whill be the top-right corner."

    def getValidMoves(self, cboard):
        self.validMoves = []
        for x in range(8):
            for y in range(8):
                if isValidMove(cboard.board, self.playType, x, y) != False:
                    self.validMoves.append([x, y])

    def set_is_contion(self):
        if 0 == len(self.validMoves):
            self._is_contion = False
        if len(self.validMoves) > 0:
            self._is_contion = True

class Computer(object):
    def __init__(self):
        self.score = 0;
        self.playType = ''
        self.move = []
        self.validMoves = []
        self._is_contion = True

    def getScoreOfBoard(self, board):
        self.score = 0
        for x in range(8):
            for y in range(8):
                if board[x][y] == self.playType:
                    self.score += 1

    def getComputerMove(self, cboard):
        possibleMoves = self.validMoves

        for x, y in possibleMoves:
            if isOnCorner(x, y):
                self.move = [x, y]

        bestScore = -1
        def makeMove(board, tile, xstart, ystart):
            tilesToFlip = isValidMove(board, tile, xstart, ystart)

            if tilesToFlip == False:
                return False

            board[xstart][ystart] = tile
            for x, y in tilesToFlip:
                board[x][y] = tile
            return True

        for x, y in possibleMoves:
            dupeBoard = getBoardCopy(cboard)
            makeMove(dupeBoard, self.playType, x, y)
            self.getScoreOfBoard(dupeBoard)
            if self.score > bestScore:
                self.move = [x, y]
                bestScore = self.score

    def getValidMoves(self, cboard):            #妈的，一样的函数居然结果不一样，明天再改
        self.validMoves = []
        for x in range(8):
            for y in range(8):
                if isValidMove(cboard.board, self.playType, x, y) != False:
                    self.validMoves.append([x, y])

    def set_is_contion(self):
        if 0 == len(self.validMoves):
            self._is_contion = False
        if len(self.validMoves) > 0:
            self._is_contion = True

class Board(object):
    def __init__(self):
        self.board = []
        self.turn = ''

    def getNewBoard(self):
        self.board = []
        for i in range(8):
            self.board.append([" "]*8)
        return self.board

    def resetBoard(self):
        for x in range(8):
            for y in range(8):
                self.board[x][y] = " "
        self.board[3][3] = "X"
        self.board[3][4] = "O"
        self.board[4][3] = "O"
        self.board[4][4] = "X"

    def whoGoesFirst(self):
        if random.randint(0, 1) == 0:
            self.turn = 'player'
        else:
            self.turn = 'computer'

    def drawBoard(self):
        NLINE = "     1   2   3   4   5   6   7   8"
        HLINE = "   +---+---+---+---+---+---+---+---+"
        print HLINE
        for y in range(8):
            print y + 1, '',
            for x in range(8):
                print "| %s"%(self.board[x][y]),
            print "|"
            print HLINE
        print NLINE

    def showPoints(self, player, computer):
        player.getScoreOfBoard(self.board)
        computer.getScoreOfBoard(self.board)
        print "You have %s points, The computer has %s points" % (player.score, computer.score)

    def makeMove(self, who):
        if isinstance(who, Player):
            xstart = int(who.move[0]) - 1
            ystart = int(who.move[1]) - 1
        elif isinstance(who, Computer):
            xstart = int(who.move[0])
            ystart = int(who.move[1])
        tilesToFlip = isValidMove(self.board, who.playType, xstart, ystart)
        if tilesToFlip == False:
            return False

        self.board[xstart][ystart] = who.playType
        for x, y in tilesToFlip:
            self.board[x][y] = who.playType
        return True

print "Welcome to black and white chess ..."
print
print "######## instruction ########"
print "Your input is valid if changing opponent's tile"
print "#############################"
print

while True:
    player = Player()
    computer = Computer()
    cboard = Board()
    cboard.getNewBoard()
    cboard.resetBoard()
    player.chooseType(computer)
    cboard.whoGoesFirst()
    while True:
        if 'player' == cboard.turn:
            player.getValidMoves(cboard)
            player.set_is_contion()
            if player._is_contion:
                cboard.drawBoard()
                cboard.showPoints(player, computer)
                player.getPlayerMove()
                if 'quit' == player.move:
                    break
                cboard.makeMove(player)
                cboard.turn = 'computer'
            else:
                break

        if 'computer' == cboard.turn:
            computer.getValidMoves(cboard)
            computer.set_is_contion()
            if computer._is_contion:
                cboard.drawBoard()
                cboard.showPoints(player, computer)
                raw_input('Press Enter to see the computer\'s move.')
                computer.getComputerMove(cboard)
                cboard.makeMove(computer)
                cboard.turn = 'player'
            else:
                break

    cboard.showPoints(player, computer)
    if player.score > computer.score:
        print "You are win!"
    elif player.score < computer.score:
        print "You are lose!"
    else:
        print "No winner!"

    while True:
        print "Do you want to play again ?  (y/n)"
        play = raw_input().lower()
        if 'y' == play:
            play = True
            break
        if 'n' == play:
            print "Thanks for playing!"
            sys.exit()