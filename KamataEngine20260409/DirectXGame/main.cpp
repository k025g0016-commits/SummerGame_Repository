#include "Fade.h"
#include "GameClearScene.h"
#include "GameOverScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"

using namespace KamataEngine;

// 現在のシーン
enum class Scene 
{
	kTitle,
	kGame,
	kGameOver,
	kGameClear,
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	// エンジンの初期化
	KamataEngine::Initialize(L"LE2C_28_ヤマモト_マサヒロ");

	// DirectXCommonのインスタンス取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 各シーン生成
	// タイトルシーン
	TitleScene* titleScene = new TitleScene();
	titleScene->Initialize();

	// ゲームシーン
	GameScene* gameScene = nullptr;

	// ゲームオーバーシーン
	GameOverScene* gameOverScene = new GameOverScene();
	gameOverScene->Initialize();

	// ゲームクリアシーン
	GameClearScene* gameClearScene = new GameClearScene();
	gameClearScene->Initialize();

	// フェード
	Fade* fade = new Fade();
	fade->Initialize();

	// 最初はタイトルシーン
	Scene currentScene = Scene::kTitle;

	// 次に移動するシーン
	Scene nextScene = Scene::kTitle;

	// シーン切り替え待ちか
	bool isSceneChanging = false;

	// フェードに使う色
	Vector4 fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

	// 最初は黒からタイトルへフェードイン
	fade->Start(Fade::Status::FadeIn, 0.5f, fadeColor);

	// メインループ
	while (true) 
	{
		// エンジンの更新
		if (KamataEngine::Update())
		{
			break;
		}

		// 入力更新
		Input::GetInstance()->Update();

		// シーン更新
		// フェードアウト中に、
		// 新しいシーン遷移要求を出さないようにする
		if (!isSceneChanging)
		{
			switch (currentScene) 
			{
			case Scene::kTitle:
			{
				titleScene->Update();

				// Spaceでゲーム開始
				if (titleScene->IsStartRequested()) 
				{
					nextScene = Scene::kGame;

					// 通常の黒フェード
					fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

					fade->Start(Fade::Status::FadeOut, 0.5f, fadeColor);

					isSceneChanging = true;
				}

				break;
			}

			case Scene::kGame:
			{
				if (gameScene != nullptr) 
				{
					gameScene->Update();

					// ポーズメニューからタイトルへ戻る
					if (gameScene->IsReturnTitleRequested())
					{
						nextScene = Scene::kTitle;

						// 黒フェード
						fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

						fade->Start(Fade::Status::FadeOut, 0.5f, fadeColor);

						isSceneChanging = true;
					}

					// ゲームオーバー
					else if (gameScene->IsGameOver())
					{
						nextScene = Scene::kGameOver;

						// 黒フェード
						fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

						fade->Start(Fade::Status::FadeOut, 0.5f, fadeColor);

						isSceneChanging = true;
					}

					// ゲームクリア
					else if (gameScene->IsGameClear())
					{
						nextScene = Scene::kGameClear;

						// クリア時だけ白フェード
						fadeColor = {1.0f, 1.0f, 1.0f, 1.0f};

						fade->Start(Fade::Status::FadeOut, 1.0f, fadeColor);

						isSceneChanging = true;
					}
				}

				break;
			}

			case Scene::kGameOver: 
			{
				gameOverScene->Update();

				// Spaceでタイトルへ戻る
				if (gameOverScene->IsReturnTitleRequested()) 
				{
					nextScene = Scene::kTitle;

					// 黒フェード
					fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

					fade->Start(Fade::Status::FadeOut, 0.5f, fadeColor);

					isSceneChanging = true;
				}

				break;
			}

			case Scene::kGameClear: 
			{
				gameClearScene->Update();

				// Spaceでタイトルへ戻る
				if (gameClearScene->IsReturnTitleRequested())
				{
					nextScene = Scene::kTitle;

					// タイトルへ戻る時は黒フェード
					fadeColor = {0.0f, 0.0f, 0.0f, 1.0f};

					fade->Start(Fade::Status::FadeOut, 0.5f, fadeColor);

					isSceneChanging = true;
				}

				break;
			}
			}
		}

		// シーン切り替え中のBGMフェード更新
		if (isSceneChanging && currentScene == Scene::kTitle) 
		{
			titleScene->UpdateBGMFade();
		}

		// フェード更新
		fade->Update();

		// フェードアウトが完了したら
		if (isSceneChanging && fade->IsFinished()) 
		{
			// タイトルから別シーンへ移動する場合
			if (currentScene == Scene::kTitle) 
			{
				titleScene->StopBGM();
			}

			// シーンを切り替える
			currentScene = nextScene;

			// 切り替え先の初期化
			switch (currentScene)
			{
			case Scene::kTitle:
			{
				titleScene->Initialize();
				break;
			}

			case Scene::kGame:
			{
				// 古いゲームシーンを削除
				delete gameScene;
				gameScene = nullptr;

				// 新しいゲーム開始
				gameScene = new GameScene();
				gameScene->Initialize();

				break;
			}

			case Scene::kGameOver: 
			{
				gameOverScene->Initialize();
				break;
			}

			case Scene::kGameClear: 
			{
				gameClearScene->Initialize();
				break;
			}
			}

			// 同じ色でフェードイン
			fade->Start(Fade::Status::FadeIn, 0.5f, fadeColor);

			isSceneChanging = false;
		}

		// 描画開始
		dxCommon->PreDraw();

		// 3Dモデル描画開始
		Model::PreDraw();

		switch (currentScene) 
		{
		case Scene::kTitle: 
		{
			titleScene->Draw();
			break;
		}

		case Scene::kGame:
		{
			if (gameScene != nullptr)
			{
				// ゲーム本体
				gameScene->Draw();
			}

			break;
		}

		case Scene::kGameOver: 
		{
			gameOverScene->Draw();
			break;
		}

		case Scene::kGameClear:
		{
			gameClearScene->Draw();
			break;
		}
		}

		// 3Dモデル描画終了
		Model::PostDraw();

		// ゲームシーンのポーズ表示
		if (currentScene == Scene::kGame && gameScene != nullptr)
		{
			// 半透明の黒を描画
			gameScene->DrawPauseDark();

			// ゲーム本体の深度情報を消す
			dxCommon->ClearDepthBuffer();

			// ポーズメニューを3D描画
			Model::PreDraw();

			gameScene->DrawPauseMenu();

			Model::PostDraw();
		}

		// 必ずシーン本体より後に描画する
		fade->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放
	delete fade;
	fade = nullptr;

	delete titleScene;
	titleScene = nullptr;

	delete gameScene;
	gameScene = nullptr;

	delete gameOverScene;
	gameOverScene = nullptr;

	delete gameClearScene;
	gameClearScene = nullptr;

	KamataEngine::Finalize();

	return 0;
}