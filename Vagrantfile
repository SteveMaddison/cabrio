#
# Vagrantfile for Cabrio FE
# =========================
#
# Provisions a development box and starts a VNC server.
#
# Quick start
# -----------
#
# On your real machine's command prompt:
#   $ vagrant up
#   $ vncviewer localhost
#
# In the resulting xterm:
#   $ make
#   $ make install
#   $ cabrio
#

# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.hostname = "cabrio.local"
  config.vm.box      = "precise-server-cloudimg-amd64-vagrant-disk1"
  config.vm.box_url  = "https://cloud-images.ubuntu.com/vagrant/precise/current/precise-server-cloudimg-amd64-vagrant-disk1.box"

  # Forwarded port mapping for access to our project(s).
  config.vm.network :forwarded_port, guest: 5900, host: 5900

  # Enable provisioning with Puppet stand alone.
  config.vm.provision :puppet do |puppet|
    puppet.manifests_path    = "puppet/manifests"
    puppet.manifest_file     = "site.pp"
    puppet.module_path       = "puppet/modules"
    puppet.facter = {
      "vagrant" => true
    }
  end
end

