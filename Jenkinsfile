echo "Start to build"

properties ([
    buildDiscarder(
        logRotator(
            artifactNumToKeepStr: '10',
            numToKeepStr: '100'
        )
    )
])

// List of targets to compile
def morpheusTargets = [
  //"LPC1768",
  //"NUCLEO_F401RE",
  //"NRF51DK",
  //"K64F"
]
// Map morpheus toolchains to compiler labels on Jenkins
def toolchains = [
  ARM: "armcc",
  // IAR: "iar_arm",
  GCC_ARM: "arm-none-eabi-gcc"
]
// yotta target includes toolchain
def yottaTargets = [
  "frdm-k64f-gcc": "gcc",
  "frdm-k64f-armcc": "armcc",
  "nrf51dk-gcc": "gcc",
  "stm32f429i-disco-gcc": "gcc",
  "x86-linux-native": "linux"
]
  
// Initial maps for parallel build steps
def stepsForParallel = [:]
// Jenkins pipeline does not support map.each, we need to use oldschool for loop
for (int i = 0; i < morpheusTargets.size(); i++) {
  for(int j = 0; j < toolchains.size(); j++) {
    def target = morpheusTargets.get(i)
    def toolchain = toolchains.keySet().asList().get(j)
    def compilerLabel = toolchains.get(toolchain)
    def stepName = "mbed-os5-${target} ${toolchain}"
    stepsForParallel[stepName] = morpheusBuildStep(target, compilerLabel, toolchain)
  }
}
// map yotta steps
for (int i = 0; i < yottaTargets.size(); i++) {
    def target = yottaTargets.keySet().asList().get(i)
    def compilerLabel = yottaTargets.get(target)
    def stepName = "mbed-os3-${target}"
    stepsForParallel[stepName] = yottaBuildStep(target, compilerLabel)
}


/* Jenkins does not allow stages inside parallel execution, 
 * https://issues.jenkins-ci.org/browse/JENKINS-26107 will solve this by adding labeled blocks
 */
// Actually run the steps in parallel - parallel takes a map as an argument, hence the above.
timestamps {
    timeout(time: 10, unit: "MINUTES") {
        parallel stepsForParallel
    }
}

def execute(cmd) {
    if(isUnix()) {
       sh "${cmd}"
    } else {
       bat "${cmd}"
    }
}

//Create morpheus build steps for parallel execution
def morpheusBuildStep(target, compilerLabel, toolchain) {
  return {
    node ("${compilerLabel}") {
      deleteDir()
      dir("mbed-client-cli") {
        def scmVars = checkout scm
        env.GIT_COMMIT_HASH = scmVars.GIT_COMMIT
        execute("mbed | grep \"^version\"")
        // does not work ?
        // execute("mbed compile -m ${target} -t ${toolchain} --library")
        dir("example/mbed-os-5") {
          // coming here: https://github.com/ARMmbed/mbed-client-cli/pull/71
          execute("mbed deploy")
          execute("mbed compile -t ${toolchain} -m ${target}")
        }
      }
    }
  }
}

//Create yotta build steps for parallel execution
def yottaBuildStep(target, compilerLabel) {
  return {
    String buildName = "mbed-os3-${target}"  
    node ("${compilerLabel}") {
      deleteDir()
      dir("mbed-client-cli") {
        def scmVars = checkout scm
        env.GIT_COMMIT_HASH = scmVars.GIT_COMMIT
      
        stage ("build:${buildName}") {  
          setBuildStatus('PENDING', "build ${buildName}", 'build starts')
          try{
              execute("yotta --version")
              execute("yotta target $target")
              execute("yotta --plain build mbed-client-cli")
              setBuildStatus('SUCCESS', "build ${buildName}", "build done")
          } catch (err) {
              echo "Caught exception: ${err}"
              if (target == "x86-linux-native") {
                postBuild()
              }
              setBuildStatus('FAILURE', "build ${buildName}", "build failed")
              throw err
          } finally {
            if (target != "x86-linux-native") {
              // clean up
              step([$class: 'WsCleanup'])
            }
          }
        } // stage
        if (target == "x86-linux-native") {  
          stage("test:${buildName}") {
            setBuildStatus('PENDING', "test ${buildName}", 'test starts')
            try {
              execute("yotta test mbed_client_cli_test")
              execute("lcov --base-directory . --directory . --capture --output-file coverage.info")
              execute("genhtml -o ./test_coverage coverage.info")
              execute("gcovr -x -o junit.xml")
              execute("cppcheck --enable=all --std=c99 --inline-suppr --template=\"{file},{line},{severity},{id},{message}\" source 2> cppcheck.txt")
              postBuild()
              setBuildStatus('SUCCESS', "test ${buildName}", "test done")
            } catch(err) {
              echo "Caught exception: ${err}"
              setBuildStatus('FAILURE', "test ${buildName}", "test failed")
              throw err
            } finally {
              // clean up
              step([$class: 'WsCleanup'])
            }
          } // stage
          /*  
          dir("example/linux") {
            // coming here: https://github.com/ARMmbed/mbed-client-cli/pull/73
            execute("make")
            execute("./cli << exit\n")
          }
          */
        } // if linux
      } // dir
    }
  }
}

def postBuild() {
    stage ("postBuild") {
        // Archive artifacts
        catchError {
            // nothing to archive
            archiveArtifacts artifacts: "cppcheck.txt"
        }

        // Publish cobertura
        catchError {
            step([
                $class: 'CoberturaPublisher',
                coberturaReportFile: 'junit.xml'
            ])
        }
        // Publish compiler warnings
        catchError {
          step([$class: 'WarningsPublisher',
                parserConfigurations: [[
                  parserName: 'GNU Make + GNU C Compiler (gcc)',
                  pattern: '**/*.c **/*.h'
                ]],
                unstableTotalAll: '0',
                useDeltaValues: true,
                usePreviousBuildAsReference: true
          ])
        }

        // Publish HTML reports
        publishHTML(target: [
            allowMissing: false,
            alwayLinkToLastBuild: false,
            keepAll: true,
            reportDir: "test_coverage",
            reportFiles: "index.html",
            reportName: "Build HTML Report"
        ])
    }
}

def setBuildStatus(String state, String context, String message) {
    step([
        $class: "GitHubCommitStatusSetter",
        reposSource: [
            $class: "ManuallyEnteredRepositorySource",
            url: "https://github.com/ARMmbed/mbed-client-cli.git"
        ],
        contextSource: [
            $class: "ManuallyEnteredCommitContextSource",
            context: context
        ],
        errorHandlers: [[
            $class: "ChangingBuildStatusErrorHandler",
            result: "UNSTABLE"
        ]],
        commitShaSource: [
            $class: "ManuallyEnteredShaSource",
            sha: env.GIT_COMMIT_HASH
        ],
        statusResultSource: [
            $class: 'ConditionalStatusResultSource',
            results: [
                [
                    $class: 'AnyBuildResult',
                    message: message,
                    state: state
                ]
            ]
        ]
    ])
}

