function changeContent (side) {
    var active_mode_title = document.getElementById("active_mode_title");
    var active_mode = document.getElementById("active_mode");

    switch(side) {
      case 1:
        active_mode_title.innerHTML= "Zen mode";
        active_mode.className= "box zen size";
        break;

      case 2:
        active_mode_title.innerHTML= "Focus mode";
        active_mode.className= "box focus size";
        break;

      case 3:
        active_mode_title.innerHTML= "Party mode";
        active_mode.className= "box party size";
        break;

      case 4:
        active_mode_title.innerHTML= "Party-hard mode";
        active_mode.className= "box party-hard size";
        break;

      case 5:
        active_mode_title.innerHTML= "Relax mode";
        active_mode.className= "box relax size";
        break;

      case 6:
        active_mode_title.innerHTML= "Initial mode";
        active_mode.className= "box initial size";
        break;
    
    window.setInterval(changeContent, 500); 
    }

    // if (side == 1) {
    //   active_mode_title.innerHTML= "Zen mode";
    //   active_mode.className= "box zen size";
    // }
    // else if (side == 1) {
    //   active_mode_title.innerHTML= "Focus mode";
    //   active_mode.className= "box focus size";
    // }
    // else if (side == 1) {
    //   active_mode_title.innerHTML= "Party mode";
    //   active_mode.className= "box party size";
    // }
    // else if (side == 1) {
    //   active_mode_title.innerHTML= "Party-hard mode";
    //   active_mode.className= "box party-hard size";
    // }
    // else if (side == 1) {
    //   active_mode_title.innerHTML= "Relax mode";
    //   active_mode.className= "box relax size";
    // }
    // else {
    //   active_mode_title.innerHTML= "Initial mode";
    //   active_mode.className= "box initial size";
    // }
    
  }

  window.setInterval(changeContent, 500);