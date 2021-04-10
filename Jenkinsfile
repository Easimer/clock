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

  stages {
    stage('Mark build as pending') {
      steps {
        setBuildStatus('Build has started', 'PENDING')
      }
    }

    stage('Configure') {
      steps {
        cmakeBuild buildType: 'Release', cleanBuild: true, installation: 'InSearchPath', buildDir: 'build'
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
        ctest workingDir: 'build', installation: 'InSearchPath'
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
