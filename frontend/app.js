const pages = document.querySelectorAll('.page');
const tabs = document.querySelectorAll('.tab');
const themeMode = document.getElementById('themeMode');
const state = { user: null, users: [], questions: [] };

function applyTheme() {
  document.body.classList.remove('theme-light', 'theme-dark');
  const mode = themeMode?.value || 'dark';
  if (mode === 'light') {
    document.body.classList.add('theme-light');
  } else {
    document.body.classList.add('theme-dark');
  }
}

themeMode?.addEventListener('change', applyTheme);
applyTheme();

tabs.forEach(t => t.addEventListener('click', () => {
  tabs.forEach(x => x.classList.remove('active'));
  t.classList.add('active');
  const id = t.dataset.page;
  pages.forEach(p => p.classList.remove('active'));
  document.getElementById(id).classList.add('active');
  if (id === 'dashboard') renderDashboard();
}));

async function loadData() {
  try {
    const [uRes, qRes] = await Promise.all([fetch('data/users.json'), fetch('data/questions.json')]);
    const usersJson = await uRes.json();
    const questionsJson = await qRes.json();
    state.users = (usersJson.users || []).map(u => ({ ...u, role: u.role || 'student', teacherApproved: u.approved || false, assignedTopics: u.assignedTopics || [] }));
    state.questions = (questionsJson.questions || []).map(q => ({ ...q, author: q.author || 'system' }));
    return true;
  } catch (err) {
    console.error(err);
    document.getElementById('home').insertAdjacentHTML('beforeend', '<p style="color: #ffcbcb;">Start the C++ app first to generate frontend JSON snapshots in <code>frontend/data</code>.</p>');
    return false;
  }
}

function byUsername(name) {
  return state.users.find(u => u.username === name);
}

function renderDashboard() {
  const profile = document.getElementById('profile');
  if (!state.user) {
    profile.innerHTML = '<p>Please login first.</p>';
    document.getElementById('leaderboard').innerHTML = '';
    document.getElementById('heatmap').innerHTML = '';
    return;
  }
  const u = byUsername(state.user);
  if (!u) {
    profile.innerHTML = '<p>User not found.</p>';
    return;
  }
  profile.innerHTML = `<strong>${u.username}</strong> &bull; ${u.tier} &bull; ${u.role || 'student'}<br>Points: ${u.points} &bull; Races: ${u.racesPlayed} &bull; Badges: ${u.badges}<br>Assigned topics: ${((u.assignedTopics || []).join(', ') || 'none')}`;
  const sorted = [...state.users].sort((a,b)=>b.points - a.points).slice(0,6);
  document.getElementById('leaderboard').innerHTML = sorted.map((x,i)=>`<div>${i+1}. ${x.username} - ${x.points} pts (${x.tier})</div>`).join('');
  const topics = ['Arrays','Math','Sorting','DP','Graphs','Greedy'];
  document.getElementById('heatmap').innerHTML = topics.map(t=>`<div>${t}: ${(Math.floor(Math.random()*70)+30)}%</div>`).join('');
}

document.getElementById('regRole').addEventListener('change', () => {
  const role = document.getElementById('regRole').value;
  document.getElementById('teacherTopicLabel').style.display = role === 'teacher' ? 'block' : 'none';
});
document.getElementById('forgotBtn').addEventListener('click', () => {
  const username = document.getElementById('loginUser').value.trim();
  const ans = prompt('Enter your security answer (favorite algorithm):');
  if (!username || !ans) return;
  const user = byUsername(username);
  if (!user) { document.getElementById('loginMsg').textContent = 'No user found'; return; }
  if (ans.trim().toLowerCase() !== (user.forgotAnswer || '').trim().toLowerCase()) {
    document.getElementById('loginMsg').textContent = 'Security answer mismatch';
    return;
  }
  document.getElementById('loginMsg').textContent = 'Please reset your password in console (demo).';
});

document.getElementById('loginBtn').addEventListener('click', async () => {
  const username = document.getElementById('loginUser').value.trim();
  const password = document.getElementById('loginPass').value.trim();
  if (!username || !password) return;
  const user = byUsername(username);
  if (user) {
    state.user = username;
    document.getElementById('loginMsg').textContent = 'Logged in! Redirecting to dashboard...';
    renderDashboard();
    tabs.forEach(t => t.classList.remove('active'));
    pages.forEach(p => p.classList.remove('active'));
    document.querySelector('[data-page="dashboard"]').classList.add('active');
    document.getElementById('dashboard').classList.add('active');
  } else {
    document.getElementById('loginMsg').textContent = 'User not found. Use C++ app to register.';
  }
});

document.getElementById('regBtn').addEventListener('click', async () => {
  const username = document.getElementById('regUser').value.trim();
  const password = document.getElementById('regPass').value.trim();
  const topic = document.getElementById('regTeachTopic').value.trim();
  const enroll = document.getElementById('regEnroll').value.trim();
  const favAlgo = document.getElementById('regFavAlgo').value.trim();
  const role = document.getElementById('regRole').value.trim();
  if (!username || !password || !enroll || !favAlgo || !role || (role==='teacher' && !topic)) {
    document.getElementById('regMsg').textContent = 'Fill all required fields (teacher needs topic).';
    return;
  }
  if (byUsername(username)) {
    document.getElementById('regMsg').textContent = 'Username exists already.';
    return;
  }
  state.users.push({ username, tier: 'Beginner', points: 0, racesPlayed: 0, badges: 0, role, favoriteSubject: role === 'teacher' ? topic : '', enrollmentId: enroll, forgotAnswer: favAlgo, teacherApproved: role !== 'teacher', assignedTopics: role === 'teacher' && topic ? [topic] : [] });
  document.getElementById('regMsg').textContent = 'Registered locally. Start C++ app to save and approve teachers.';
});

document.getElementById('runRace').addEventListener('click', () => {
  if (!state.user) {
    document.getElementById('raceOutput').innerHTML = '<p class="msg">Login first to race.</p>';
    return;
  }
  const topic = document.getElementById('raceTopic').value.trim();
  if (!topic) {
    document.getElementById('raceOutput').innerHTML = '<p class="msg">Enter a topic to race.</p>';
    return;
  }

  const me = state.users.find(u => u.username === state.user);
  if (!me) return;

  // Match same interest: use teachers assigned to topic or users who have questions in topic
  const eligible = state.users.filter(u => u.username !== me.username && (u.assignedTopics?.includes(topic) || u.role === 'student'));
  if (!eligible.length) {
    document.getElementById('raceOutput').innerHTML = `<p class="msg">No same-interest opponents available for topic ${topic}. Ask teachers to add questions.</p>`;
    return;
  }

  const partnerName = document.getElementById('racePartner').value.trim();
  let partner = null;
  if (partnerName) {
    partner = state.users.find(u => u.username === partnerName);
    if (!partner || partner.username === me.username) {
      document.getElementById('raceOutput').innerHTML = '<p class="msg">Invalid partner username; choose another or leave empty.</p>';
      return;
    }
    if (!eligible.some(u => u.username === partner.username)) {
      document.getElementById('raceOutput').innerHTML = `<p class="msg">Partner does not share topic interest ${topic}.</p>`;
      return;
    }
  } else {
    partner = eligible[Math.floor(Math.random() * eligible.length)];
  }

  const pool = state.questions.filter(q => q.topic.toLowerCase() === topic.toLowerCase());
  if (!pool.length) {
    document.getElementById('raceOutput').innerHTML = `<p class="msg">No questions for topic ${topic}. Have a teacher add them first.</p>`;
    return;
  }

  const raceQs = [];
  for (let i = 0; i < 3; i++) {
    raceQs.push(pool[Math.floor(Math.random() * pool.length)]);
  }

  const players = [me, partner];
  const results = players.map(p => {
    let totalTime = 0;
    let points = 0;
    raceQs.forEach(q => {
      const t = 12 + Math.floor(Math.random() * 20) + q.difficulty * 8;
      totalTime += t;
      points += Math.max(5, 140 - t + q.difficulty * 10);
    });
    return { user: p.username, totalTime, points };
  });

  results.sort((a, b) => a.totalTime - b.totalTime);
  let output = `<div><strong>Race on topic: ${topic}</strong></div>`;
  output += `<div>Partner: ${partner.username} (same interest)</div>`;
  results.forEach((r, idx) => {
    output += `<div>${idx+1}. ${r.user} - ${r.points} pts - ${r.totalTime}s</div>`;
  });
  output += `<div class="msg">Winner: ${results[0].user}</div>`;
  document.getElementById('raceOutput').innerHTML = output;

  results.forEach(r => {
    const user = state.users.find(u => u.username === r.user);
    if (user) {
      user.points = (user.points || 0) + r.points;
      user.racesPlayed = (user.racesPlayed || 0) + 1;
      if (r.totalTime < 65) user.badges = (user.badges || 0) + 1;
    }
  });
  renderDashboard();
});

loadData();
document.querySelector('.hero-actions .primary').addEventListener('click', () => { tabs.forEach(t => t.classList.remove('active')); pages.forEach(p => p.classList.remove('active')); document.querySelector('[data-page="register"]').classList.add('active'); document.getElementById('register').classList.add('active'); });

document.getElementById('startFlow')?.addEventListener('click', () => { tabs.forEach(t => t.classList.remove('active')); pages.forEach(p => p.classList.remove('active')); document.querySelector('[data-page="register"]').classList.add('active'); document.getElementById('register').classList.add('active'); });
document.getElementById('viewSteps')?.addEventListener('click', () => { alert('1) Register\n2) Login\n3) Go to Dashboard\n4) Start Race and track progress.'); });
document.querySelectorAll('[data-action]').forEach(el => { el.addEventListener('click', () => {
  const target = el.dataset.action;
  tabs.forEach(t => t.classList.remove('active'));
  pages.forEach(p => p.classList.remove('active'));
  document.querySelector(`[data-page="${target}"]`)?.classList.add('active');
  document.getElementById(target)?.classList.add('active');
});});

function renderTeacherPage() {
  const msg = document.getElementById('teacherMsg');
  const list = document.getElementById('teacherQuestions');
  if (!state.user) { msg.textContent = 'Login first'; list.innerHTML = ''; return; }
  const u = byUsername(state.user);
  if (!u || u.role !== 'teacher') { msg.textContent = 'Only teachers can add questions'; list.innerHTML = ''; return; }
  if (!u.teacherApproved) { msg.textContent = 'Teacher pending approval'; list.innerHTML = ''; return; }
  const topicHint = (u.assignedTopics || []).join(', ') || 'None assigned by admin yet';
  msg.textContent = 'Add questions only for your assigned topics: ' + topicHint;
  const own = state.questions.filter(q => q.author === u.username);
  list.innerHTML = own.length ? own.map(q => `<div class="card"><strong>${q.topic}</strong> (${q.type})<br>${q.prompt}<br><small>${q.author} | diff ${q.difficulty}</small></div>`).join('') : '<p>No questions yet.</p>';
}

function renderAdminPage() {
  const panel = document.getElementById('adminApprovals');
  const pending = state.users.filter(u => u.role === 'teacher' && !u.teacherApproved);
  if (!pending.length) { panel.innerHTML = '<p>No pending teacher approvals.</p>'; return; }
  panel.innerHTML = pending.map(u => `<div class="card">${u.username} <button class="secondary approve" data-user="${u.username}">Approve</button></div>`).join('');
  panel.querySelectorAll('.approve').forEach(btn => {
    btn.addEventListener('click', () => {
      const u = byUsername(btn.dataset.user);
      if (u) {
        u.teacherApproved = true;
        btn.closest('.card').innerHTML = `${u.username} approved.`;
      }
    });
  });
}

document.getElementById('teacherAdd')?.addEventListener('click', () => {
  if (!state.user) return;
  const u = byUsername(state.user);
  if (!u || u.role !== 'teacher' || !u.teacherApproved) {
    document.getElementById('teacherMsg').textContent = 'Only approved teachers add questions.';
    return;
  }
  const topic = document.getElementById('teacherTopic').value.trim();
  const type = document.getElementById('teacherType').value;
  const prompt = document.getElementById('teacherPrompt').value.trim();
  const answer = document.getElementById('teacherAnswer').value.trim();
  const difficulty = parseInt(document.getElementById('teacherDifficulty').value, 10);
  if (!topic || !prompt || !answer || !difficulty) { document.getElementById('teacherMsg').textContent = 'Fill all fields.'; return; }
  const allowed = (u.assignedTopics || []).map(x => x.toLowerCase());
  if (allowed.length > 0 && !allowed.includes(topic.toLowerCase())) {
    document.getElementById('teacherMsg').textContent = 'Topic not assigned by admin. Ask admin to assign this topic.';
    return;
  }
  const duplicate = state.questions.some(q => q.topic.toLowerCase() === topic.toLowerCase() && q.prompt === prompt);
  if (duplicate) {
    document.getElementById('teacherMsg').textContent = 'Question already exists for this topic. Ignored.';
    return;
  }
  const nextId = (state.questions[state.questions.length-1]?.id || 0) + 1;
  state.questions.push({ id: nextId, topic, type, prompt, answer, author: u.username, difficulty });
  document.getElementById('teacherMsg').textContent = 'Question added.';
  renderTeacherPage();
});

document.querySelector('[data-page="teacher"]').addEventListener('click', renderTeacherPage);
document.querySelector('[data-page="admin"]').addEventListener('click', renderAdminPage);

