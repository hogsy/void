#ifndef VOID_CLIENT_STATE
#define VOID_CLIENT_STATE

class CWorld;
class CBuffer;
class CCamera;

class  CSoundManager;
class  CMusic;
class CClient;
struct I_HudRenderer;


/*
================================================
Maintains clients game state
Nearly all cleint side systems act on this data
================================================
*/
class CClientState
{
public:
	CClientState(CClient	   & rClient,
				 I_HudRenderer * pHud,
				 CSoundManager * pSound,
				 CMusic		   * pMusic);

	~CClientState();

	//spawn for the first time.
	void BeginGame();
	
	bool LoadWorld(CWorld * pWorld);
	void UnloadWorld();

	void RunFrame(float frameTime);
	void WriteCmdUpdate(CBuffer &buf);
	void UpdateView();

	//==================================================
	//Movement
	void Move(vector_t &dir, float time);
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void RotateRight(const float &val);
	void RotateLeft(const float &val);
	void RotateUp(const float &val);
	void RotateDown(const float &val);
	void CamPath();

	//==================================================
	//Client side stuff
	I_HudRenderer * m_pHud;
	CSoundManager * m_pSound;
	CMusic		  * m_pMusic;

	CWorld	 *  m_pWorld;
	CClient	 &	m_rClient;

//	float		m_fFrameTime;
	bool		m_ingame;

	int			m_numEnts;
	ClEntity 	m_entities[GAME_MAXENTITIES];
	ClClient 	m_clients[GAME_MAXCLIENTS];

	ClClient *	m_pGameClient;
	
	ClCmd		m_cmd;
	ClCmd		m_oldCmd;

	//This should hook up to the game client whne the client
	//enters a game
	CCamera	*	m_pCamera;

	vector_t	m_screenBlend;
	vector_t	desired_movement;
	
	int			m_campath;
	float		m_camtime;
	float		m_maxvelocity;

	void Spawn(vector_t	*origin, vector_t *angles);
};

#endif