let x = 0, y = 0, z = 0;
let ax = 0, ay = 0, az = 0;
let history = [];
const maxHistory = 400;

// 3D visualization display
const sketch3D = (p) => {
  let socket;

  p.setup = () => {
    let canvas = p.createCanvas(600, 450, p.WEBGL);
    canvas.parent("sketch-3d");
    
    socket = new WebSocket("ws://localhost:8080");
    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      x = data.xd; y = data.yd; z = data.zd;
      ax = data.xa; ay = data.ya; az = data.za;
      
      // Store the point for both the trail and the graph
      history.push(p.createVector(x, y, z));
      if (history.length > maxHistory) history.shift();
    };
  };

  p.draw = () => {
    p.background(240);
    p.ambientLight(100);
    p.directionalLight(255, 255, 255, 0, 0, -1);
    p.ortho(-p.width/2, p.width/2, p.height/2, -p.height/2, -2000, 2000);
    p.camera(800, 800, 800, 0, 0, 0, 0, 0, 1);

    // persistent elements
    drawGrid(p);
    drawAxes(p, 150);
    drawTrail(p);

    // moving blob
    p.push();
      p.translate(x, y, z);
      p.noStroke();
      p.ambientMaterial(100, 150, 255);
      p.box(20, 20, 10);
    p.pop();

    updateHTML();
  };
};

// 2D graph
const sketchGraph = (p) => {
  p.setup = () => {
    let canvas = p.createCanvas(400, 450);
    canvas.parent("sketch-graph");
  };

  p.draw = () => {
    p.background(255);
    p.stroke(230);
    p.line(0, p.height/2, p.width, p.height/2); // Baseline

    if (history.length < 2) return;

    let scaleY = 0.5;
    let centerY = p.height / 2;

    for (let i = 1; i < history.length; i++) {
      let x1 = p.map(i - 1, 0, maxHistory, 0, p.width);
      let x2 = p.map(i, 0, maxHistory, 0, p.width);

      p.strokeWeight(2);
      // X - red; Y - green; Z - blue
      p.stroke(255, 0, 0);
      p.line(x1, centerY - history[i-1].x * scaleY, x2, centerY - history[i].x * scaleY);
      p.stroke(0, 180, 0);
      p.line(x1, centerY - history[i-1].y * scaleY, x2, centerY - history[i].y * scaleY);
      p.stroke(0, 0, 255);
      p.line(x1, centerY - history[i-1].z * scaleY, x2, centerY - history[i].z * scaleY);
    }
    p.noStroke(); p.fill(0); p.text("Real-time XYZ (cm)", 10, 20);
  };
};

// Initialize both sketches
new p5(sketch3D);
new p5(sketchGraph);

function drawTrail(p) {
  p.noFill();
  p.stroke(255, 0, 0);
  p.strokeWeight(2);
  p.beginShape();
  for (let v of history) {
    p.vertex(v.x, v.y, v.z);
  }
  p.endShape();
}

function drawAxes(p, len) {
  p.strokeWeight(3);
  p.stroke(255, 0, 0); p.line(0, 0, 0, len, 0, 0); // X
  p.stroke(0, 255, 0); p.line(0, 0, 0, 0, len, 0); // Y
  p.stroke(0, 0, 255); p.line(0, 0, 0, 0, 0, len); // Z
}

function drawGrid(p) {
  p.stroke(200); p.strokeWeight(1);
  for (let i = -500; i <= 500; i += 50) {
    p.line(i, -500, 0, i, 500, 0);
    p.line(-500, i, 0, 500, i, 0);
  }
}

function updateHTML() {
  const posDiv = document.getElementById("positionDisplay");
  const accDiv = document.getElementById("acceleraDisplay");
  if(posDiv) posDiv.innerText = `POS | X: ${x.toFixed(1)} Y: ${y.toFixed(1)} Z: ${z.toFixed(1)}`;
  if(accDiv) accDiv.innerText = `ACC | X: ${ax.toFixed(1)} Y: ${ay.toFixed(1)} Z: ${az.toFixed(1)}`;
}
