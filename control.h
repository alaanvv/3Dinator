void handle_key(SDL_Event ev) {
  switch (ev.key.keysym.sym) {
    case 97: // A
      cam.pos[0] -= cos(cam.ang[0]) * SPEED;
      cam.pos[2] -= sin(cam.ang[0]) * SPEED;
      break;
    case 100: // D
      cam.pos[0] += cos(cam.ang[0]) * SPEED;
      cam.pos[2] += sin(cam.ang[0]) * SPEED;
      break;
    case 119: // W
      cam.pos[0] -= sin(cam.ang[0]) * SPEED;
      cam.pos[2] += cos(cam.ang[0]) * SPEED;
      break;
    case 115: // S
      cam.pos[0] += sin(cam.ang[0]) * SPEED;
      cam.pos[2] -= cos(cam.ang[0]) * SPEED;
      break;
    case 101: // E
      cam.pos[1] += SPEED;
      break;
    case 113: // Q
      cam.pos[1] -= SPEED;
      break;
    case 1073741904: // LEFT
      cam.ang[0] += CAMERA_SPEED;
      break;
    case 1073741903: // RIGHT
      cam.ang[0] -= CAMERA_SPEED;
      break;
    case 1073741906: // UP
      cam.ang[1] = MIN(cam.ang[1] + CAMERA_SPEED, CAMERA_Y_LOCK);
      break;
    case 1073741905: // DOWN
      cam.ang[1] = MAX(cam.ang[1] - CAMERA_SPEED, -CAMERA_Y_LOCK);
      break;
    case 102: // F
      printf("POS X %.2f Y %.2f Z %.2f\nANG Y %.2f X %.2f\n\n", cam.pos[0], cam.pos[1], cam.pos[2], cam.ang[0] / PI, cam.ang[1] / PI);
      break;
  }
} 
