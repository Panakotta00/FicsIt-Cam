pipeline {
    agent {
        label 'Windows2019'
    }

    options {
        disableConcurrentBuilds()
        skipDefaultCheckout(true)
    }

	environment {
        SML_BRANCH = 'master'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm: [
                    $class: 'GitSCM',
                    branches: scm.branches,
                    extensions: [[
                        $class: 'RelativeTargetDirectory',
                        relativeTargetDir: 'FicsItCam'
                    ]],
                    submoduleCfg: scm.submoduleCfg,
                    doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
                    userRemoteConfigs: scm.userRemoteConfigs
                ]
            }
        }

        stage('Setup UE4') {
            steps {
                dir('ue4') {
                    withCredentials([string(credentialsId: 'GitHub-API', variable: 'GITHUB_TOKEN')]) {
                        retry(3) {
                            bat label: '', script: 'github-release download --user SatisfactoryModdingUE --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.zip" > UnrealEngine-CSS-Editor-Win64.zip'
                        }
                        bat label: '', script: '7z x UnrealEngine-CSS-Editor-Win64.zip'
                    }
                    bat label: '', script: 'SetupScripts\\Register.bat'
                }
            }
        }
        
        stage('Setup WWise & Rider') {
            steps {
                dir("FicsItCam") {
                    bat label: '', script: '7z x %WWISE_PLUGIN% -oPlugins\\'
                }
            }
        }
        
        stage('Setup SML') {
            steps {
                bat label: '', script: 'git clone --branch %SML_BRANCH% https://github.com/satisfactorymodding/SatisfactoryModLoader.git'
                bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Source\\FactoryGame FicsItCam\\Source\\FactoryGame > copy.log'
                bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Source\\SML FicsItCam\\Source\\SML > copy.log'
                bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Plugins\\Alpakit FicsItCam\\Plugins\\Alpakit > copy.log'
				bat label: '', script: 'dir FicsItCam\\Plugins'
				bat label: '', script: 'dir FicsItCam\\Plugins\\Alpakit'
                bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Content\\SML FicsItCam\\Content\\SML > copy.log'
            }
        }

        stage('Build FicsItCam') {
            steps {
                bat label: '', script: '.\\ue4\\Engine\\Binaries\\DotNET\\UnrealBuildTool.exe  -projectfiles -project="%WORKSPACE%\\FicsItCam\\FactoryGame.uproject" -game -rocket -progress'
                bat label: '', script: 'MSBuild.exe .\\FicsItCam\\FactoryGame.sln /p:Configuration="Shipping" /p:Platform="Win64" /t:"Games\\FactoryGame"'
                bat label: '', script: 'MSBuild.exe .\\FicsItCam\\FactoryGame.sln /p:Configuration="Development Editor" /p:Platform="Win64" /t:"Games\\FactoryGame"'
                retry(3) {
                    bat label: '', script: '.\\ue4\\Engine\\Build\\BatchFiles\\RunUAT.bat BuildCookRun -nop4 -project="%WORKSPACE%\\FicsItCam\\FactoryGame.uproject" -cook -skipstage -iterate'
                }
                
                dir('ToPak/FactoryGame/Content') {
                    bat label: '', script: 'xcopy /Y /E /I %WORKSPACE%\\FicsItCam\\Saved\\Cooked\\WindowsNoEditor\\FactoryGame\\Content\\FicsItCam .\\FicsItCam > copy.log'
                }
                dir('ToPak') {
                    bat label: '', script: 'copy %U4PAK% .'
                    bat label: '', script: 'python .\\u4pak.py pack FicsItCam.pak FactoryGame'
                    bat label: '', script: 'rmdir /S /Q .\\FactoryGame'
                }
                dir('ToPak/FactoryGame/Content') {
                    bat label: '', script: 'xcopy /Y /E /I %WORKSPACE%\\FicsItCam\\Saved\\Cooked\\WindowsNoEditor\\FactoryGame\\Content\\SML .\\SML > copy.log'
                }
                dir('ToPak') {
                    bat label: '', script: 'copy %U4PAK% .'
                    bat label: '', script: 'python .\\u4pak.py pack SML.pak FactoryGame'
                }
            }
        }

        stage('Package FicsItCam') {
            when {
                not {
                    changeRequest()
                }
            }
            steps {
                dir('FicsItCam') {
                    bat label: '', script: "7z a -tzip -mx9 -mm=LZMA -xr!*.pdb ..\\FicsItCam-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip Binaries\\ Config\\ Content\\ Library\\ Plugins\\Alpakit\\ Source\\ .gitattributes .gitignore FactoryGame.uproject LICENSE README.md ..\\ToPak"
                    bat label: '', script: 'copy .\\Binaries\\Win64\\UE4-FicsItCam-Win64-Shipping.* .\\'
                    bat label: '', script: 'copy ..\\ToPak\\FicsItCam.pak .\\'
                    bat label: '', script: "7z a -tzip -mx9 FicsItCam.smod .\\data.json .\\FicsItCam.bmp .\\UE4-FicsItCam-Win64-Shipping.dll .\\UE4-FicsItCam-Win64-Shipping.pdb .\\FicsItCam.pak"
                }
                bat label: '', script: '7z x %EXTRACT_AND_GO% -o.\\'
                bat label: '', script: 'copy .\\FicsItCam\\Binaries\\Win64\\UE4-SML-Win64-Shipping.* .\\ExtractAndGo\\loaders\\'
                bat label: '', script: 'copy .\\ToPak\\SML.pak .\\ExtractAndGo\\loaders\\'
                bat label: '', script: 'copy .\\FicsItCam\\Binaries\\Win64\\UE4-FicsItCam-Win64-Shipping.* .\\ExtractAndGo\\mods\\'
                bat label: '', script: 'copy .\\ToPak\\FicsItCam.pak .\\ExtractAndGo\\mods\\'
                bat label: '', script: "7z a -tzip -mx9 -mm=LZMA FicsItCam-ExtractAndGo-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip .\\ExtractAndGo\\*"
                archiveArtifacts artifacts: "FicsItCam-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip", fingerprint: true, onlyIfSuccessful: true
                archiveArtifacts artifacts: "FicsItCam-ExtractAndGo-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip", fingerprint: true, onlyIfSuccessful: true
                archiveArtifacts artifacts: 'FicsItCam\\Binaries\\Win64\\UE4-FicsItCam-Win64-Shipping.dll', fingerprint: true, onlyIfSuccessful: true
                archiveArtifacts artifacts: 'ToPak\\FicsItCam.pak', fingerprint: true, onlyIfSuccessful: true
                archiveArtifacts artifacts: 'FicsItCam\\FicsItCam.smod', fingerprint: true, onlyIfSuccessful: true
            }
        }
    }
    post {
        always {
            cleanWs()
        }
    }
}