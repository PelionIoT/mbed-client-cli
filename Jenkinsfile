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
  "K64F"
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
  timeout(time: 30, unit: "MINUTES") {
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
        String buildName = "mbed-os5-${target}-${toolchain}"
        def scmVars = checkout scm
        env.GIT_COMMIT_HASH = scmVars.GIT_COMMIT
        setBuildStatus('PENDING', "build ${buildName}", 'build starts')
        stage ("build:${buildName}") {
          try{
            execute("mbed --version")
            execute("echo https://github.com/armmbed/mbed-os/#62f8b922b420626514fd4690107aff4188469833 > mbed-os.lib")
            execute("mbed deploy")
            execute("mbed compile -m ${target} -t ${toolchain} --library")
            setBuildStatus('SUCCESS', "build ${buildName}", "build done")
          } catch (err) {
            echo "Caught exception: ${err}"
            setBuildStatus('FAILURE', "build ${buildName}", "build failed")
            throw err
          }
        }
        stage("build:example:${buildName}") {
          execute("mkdir ../example-mbed-os-5 || true")
          execute("cp -R example/mbed-os-5 ../example-mbed-os-5")
          dir("../example-mbed-os-5") {
            def exampleName = "example-${buildName}"
            setBuildStatus('PENDING', "build ${exampleName}", 'build starts')
            try {
              execute("echo \"https://github.com/ARMmbed/mbed-os/#62f8b922b420626514fd4690107aff4188469833\" > mbed-os.lib")
              execute("echo \"https://github.com/ARMmbed/mbed-client-cli#${env.GIT_COMMIT_HASH}\" > mbed-clint-cli.lib")
              execute("mbed new .")
              execute("mbed deploy")
              execute("rm -rf ./mbed-os/features/frameworks/mbed-client-cli")
              execute("mbed compile -t ${toolchain} -m ${target} --stats-depth=10")
              execute("cp --parents `find -name example-mbed-os-5.bin` ../mbed-client-cli")
              // compile minimum lib
              execute("mbed compile -t ${toolchain} -m ${target} --stats-depth=10 -DMBED_CMDLINE_MINIMUM_CONFIG=1")
              execute("mkdir -p ../mbed-client-cli/output/minimal")
              execute("cp --parents `find -name example-mbed-os-5.bin` ../mbed-client-cli/output/minimal")

              setBuildStatus('SUCCESS', "build ${exampleName}", "build done")
            } catch(err) {
              echo "Caught exception: ${err}"
              setBuildStatus('FAILURE', "build ${exampleName}", "build failed")
              currentBuild.result = 'FAILURE'
            } finally {
              // clean up
              postBuild(buildName, false)
              step([$class: 'WsCleanup'])
            }
          }
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
        def isTest = target == "x86-linux-native" // tests are valid only in linux target
        stage ("build:${buildName}") {
          setBuildStatus('PENDING', "build ${buildName}", 'build starts')
          try{
            execute("yotta --version")
            execute("yotta target $target")
            execute("yotta --plain build mbed-client-cli")
            setBuildStatus('SUCCESS', "build ${buildName}", "build done")
          } catch (err) {
            echo "Caught exception: ${err}"
            setBuildStatus('FAILURE', "build ${buildName}", "build failed")
            currentBuild.result = 'FAILURE'
          }
        } // stage
        if (isTest) {
          stage("test:${buildName}") {
            setBuildStatus('PENDING', "test ${buildName}", 'test starts')
            try {
              execute("yotta test mbed_client_cli_test")
              execute("lcov --base-directory . --directory . --capture --output-file coverage.info")
              execute("genhtml -o ./test_coverage coverage.info")
              execute("gcovr -x -o junit.xml")
              execute("cppcheck --enable=all --std=c99 --inline-suppr --template=\"{file},{line},{severity},{id},{message}\" source 2> cppcheck.txt")
              setBuildStatus('SUCCESS', "test ${buildName}", "test done")
            } catch(err) {
              echo "Caught exception: ${err}"
              setBuildStatus('FAILURE', "test ${buildName}", "test failed")
              currentBuild.result = 'FAILURE'
            }
          } // stage
          stage("example:${buildName}") {
            dir("example/linux") {
              def exampleName = "example-linux"
              setBuildStatus('PENDING', "build ${exampleName}", 'build starts')
              try {
                execute("make")
                setBuildStatus('SUCCESS', "build ${exampleName}", "build done")
              } catch(err) {
                echo "Caught exception: ${err}"
                setBuildStatus('FAILURE', "build ${exampleName}", "build failed")
                currentBuild.result = 'FAILURE'
              }
            }
          } // stage
          stage("leak-check:${buildName}") {
            dir("example/linux") {
              def stageName = "leak-check"
              setBuildStatus('PENDING', "test ${stageName}", 'test starts')
              try {
                execute("./memtest.sh")
                setBuildStatus('SUCCESS', "test ${stageName}", "test done")
              } catch(err) {
                echo "Caught exception: ${err}"
                setBuildStatus('FAILURE', "test ${stageName}", "test failed")
                currentBuild.result = 'FAILURE'
              }
            }
          } // stage
        } // if linux
        postBuild(buildName, isTest)
        step([$class: 'WsCleanup'])
      } // dir
    }
  }
}

def postBuild(buildName, isTest) {
    // move files to target+toolchain specific folder
    execute("mkdir -p output/${buildName}")
    execute("find . -name 'libmbed-client-cli.a' -exec mv {} 'output/${buildName}' \\;")
    execute("find . -name 'mbed-client-cli.ar' -exec mv {} 'output/${buildName}' \\;")
    execute("find . -name 'example-mbed-os-5.bin' -exec mv {} 'output/${buildName}' \\;")

    // Archive artifacts
    step([
      $class: 'ArtifactArchiver',
      artifacts: "cppcheck.txt,output/**",
      fingerprint: true,
      allowEmptyArchive: true
    ])
    if (isTest) {
        // Publish cobertura
        step([
            $class: 'CoberturaPublisher',
            coberturaReportFile: 'junit.xml'
        ])
        // Publish compiler warnings
        step([
          $class: 'WarningsPublisher',
          parserConfigurations: [[
            parserName: 'GNU Make + GNU C Compiler (gcc)',
            pattern: 'mbed-client-cli/*.c,source/*.h,test/*.cpp'
          ]],
          unstableTotalAll: '0',
          useDeltaValues: true,
          usePreviousBuildAsReference: true
        ])
        // Publish HTML reports
        publishHTML(target: [
          alwayLinkToLastBuild: false,
          keepAll: true,
          reportDir: "test_coverage",
          reportFiles: "index.html",
          reportName: "Build HTML Report"
        ])
    }
}
// helper function to set build status to github PR
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
