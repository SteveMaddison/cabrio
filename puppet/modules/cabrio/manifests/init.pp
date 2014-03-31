# === Class cabrio
#
# Module to set up Carbio FE development environment.
#
# === Authors
#
# Steve Maddison <steve@cosam.org>
#
# === Copyright
#
# Copyright 2014 Steve Maddison
#
class cabrio (
) {
  case $::operatingsystem {
    /^(Debian|Ubuntu)$/: {
      $packages = [
        'gcc', 'gdb',
        'make',
        'xserver-xorg', 'xvfb', 'x11vnc',
        'libc6-dev',
        'libavcodec-dev', 'libavutil-dev', 'libavformat-dev',
        'libswscale-dev',
        'freeglut3-dev',
        'libsdl1.2-dev', 'libsdl-image1.2-dev', 'libsdl-gfx1.2-dev',
        'libsdl-mixer1.2-dev', 'libsdl-ttf2.0-dev',
        'libxml2-dev',
        'zlib1g-dev',
      ]
    }
    default: {
      warning("Operating system '$::operatingsystem' not supported.")
    }
  }

  package { $packages: }

  if $vagrant {
    # Link to the sources.
    file { '/home/vagrant/cabrio':
      ensure => link,
      target => '/vagrant',
    }

    # Start VNC server unless already listening.
    exec { 'vnc':
      path    => ['/usr/bin','/bin'],
      command => 'nohup x11vnc -create &',
      unless  => 'netstat -tln | awk \'{ print $4 }\' | grep :5900',
      user    => 'vagrant',
    }
    Package<||> -> Exec['vnc']
  }
}

