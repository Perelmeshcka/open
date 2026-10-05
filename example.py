import cr3


def knightdo(me: Character):
  for ch in spawned:
    if ch.id == me.id:
      continue
    if ch.type == MONSTER:
      dealdmg(me, ch, 12)


knightstates = ["IDLE", "WALK", "DAMAGED"]


if __name__ == '__main__':
  exanimlist = initanimlist("tex/knight.png", nanim=3, len=[2, 3, 1])
  
  knight = initchar(exanimlist, knightdo, knightstates)
  spawnchar(knight)

  start()
