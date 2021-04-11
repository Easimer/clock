void setBuildStatus(String message, String state) {
  step([
    $class: 'GitHubCommitStatusSetter',
    reposSource: [$class: 'ManuallyEnteredRepositorySource', url: 'https://github.com/Easimer/clock'],
    contextSource: [$class: 'ManuallyEnteredCommitContextSource', context: 'ci/jenkins/build-status'],
    errorHandlers: [[$class: 'ChangingBuildStatusErrorHandler', result: 'UNSTABLE']],
    statusResultSource: [ $class: 'ConditionalStatusResultSource', results: [[$class: 'AnyBuildResult', message: message, state: state]] ]
  ])
}

pipeline {
  agent any

  options {
    buildDiscarder(logRotator(numToKeepStr: '30', artifactNumToKeepStr: '2'))
  }

  environment {
    CFLAGS   = '-g -O0 -Wall -W -fprofile-arcs -ftest-coverage'
    CXXFLAGS = '-g -O0 -Wall -W -fprofile-arcs -ftest-coverage'
    LDFLAGS  = '-fprofile-arcs -ftest-coverage'
  }

  stages {
    stage('Mark build as pending') {
      steps {
        setBuildStatus('Build has started', 'PENDING')
      }
    }

    stage('Configure') {
      steps {
        cmakeBuild buildType: 'Debug', cleanBuild: true, installation: 'InSearchPath', buildDir: 'build'
      }
    }

    stage('Build') {
      steps {
        cmakeBuild buildDir: 'build', installation: 'InSearchPath', steps: [ [args: 'all'] ]
      }
    }

    stage('Gather warnings') {
      steps {
        recordIssues(tools: [clang()])
      }
    }

    stage('Run tests') {
      steps {
        sh 'scripts/run_tests.sh'
      }
    }
    stage('Gather coverage info') {
      steps {
        cobertura autoUpdateHealth: false, autoUpdateStability: false, coberturaReportFile: 'out/coverage.xml', conditionalCoverageTargets: '70, 0, 0', failUnhealthy: false, failUnstable: false, lineCoverageTargets: '80, 0, 0', maxNumberOfBuilds: 0, methodCoverageTargets: '80, 0, 0', onlyStable: false, zoomCoverageChart: false
      }
    }
  }
  post {
    success {
      setBuildStatus("Build was successful", "SUCCESS");
    }
    failure {
      setBuildStatus("Build failed", "FAILURE");
    }
  }
}
