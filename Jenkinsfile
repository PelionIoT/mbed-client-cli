// List of targets to compile
def morpheusTargets = [
  //"LPC1768",
  //"NUCLEO_F401RE",
  //"NRF51822",
  "K64F"
  ]
  
// Map morpheus toolchains to compiler labels on Jenkins
def toolchains = [
  ARM: "armcc",   // open issue: https://github.com/ARMmbed/mbed-os/issues/125
  // IAR: "iar_arm", // open issue: https://github.com/ARMmbed/mbed-os/issues/357
  GCC_ARM: "arm-none-eabi-gcc"
  ]
  
// Initial maps for parallel build steps
def stepsForParallel = [:]

// Jenkins pipeline does not support map.each, we need to use oldschool for loop
for (int i = 0; i < morpheusTargets.size(); i++) {
  for(int j = 0; j < toolchains.size(); j++) {
    def target = morpheusTargets.get(i)
    def toolchain = toolchains.keySet().asList().get(j)
    def compilerLabel = toolchains.get(toolchain)
    def stepName = "${target} ${toolchain}"
    stepsForParallel[stepName] = morpheusBuildStep(target, compilerLabel, toolchain)
  }
}

/* Jenkins does not allow stages inside parallel execution, 
 * https://issues.jenkins-ci.org/browse/JENKINS-26107 will solve this by adding labeled blocks
 */
// Actually run the steps in parallel - parallel takes a map as an argument, hence the above.
stage "build testapps"
parallel stepsForParallel

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
        checkout scm
        execute("mbed | grep \"^version\"")
        execute("mbed deploy --protocol ssh")
        dir("mbed-os") {
          deleteDir()
          git url: 'git@github.com:ARMmbed/mbed-os'
          execute("mbed deploy --protocol ssh")
          execute("echo mbed-os revision:")
          execute("git rev-parse HEAD")
        }

        // workaround because of this: https://github.com/ARMmbed/mbed-os/issues/125
        if(isUnix()) {
           execute("cp /builds/scripts/mbed_settings.py .")
        } else {
           execute("cp C:/mbed_tools/scripts/mbed_settings.py .")
        }
        execute("mbed compile -m ${target} -t ${toolchain} --library")
      }
    }
  }
}
