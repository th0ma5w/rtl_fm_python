/** @jsx React.DOM */

var ValuePair = React.createClass({
	render: function(){
		return (
			<div>{this.props.key} {': '} {this.props.value}</div>
			)
	}
});

var SignalMeter = React.createClass({
	render: function(){
		var signalStyle={
			'background-color': "black",
			width: this.props.signal,
			height: '5px',
			display: "block"
		}
		return (<div>
			<div><small>{this.props.signal}</small></div>
			<div style={signalStyle}> </div>
			</div>
			)
	}
});

var FrequencyForm = React.createClass({
	handleSubmit: function(){
		var freq=this.refs.freq.getDOMNode().value.trim();
		$.ajax({
			url: '/frequency/human/' + freq,
			dataType: 'json'		
		});
		return false;
	},
	render: function(){
		return(
			<form className="FrequencyForm" onSubmit={this.handleSubmit}>
				<input type="text" placeholder={this.props.freq} ref="freq" />
			</form>
		);
	}
}
);

var FrequencyDisplay = React.createClass({
	render: function(){
		return (
			<h1>{this.props.freq}</h1>
			)
	}
});

var AutoGainEnabled = React.createClass({
	handleSubmit: function(){
		var autogain=this.refs.autogain.getDOMNode().value.trim();
		if (autogain=='on'){
			$.ajax({
				url: '/gain/auto',
				dataType: 'json'
			});
		}
	},
	render:	function(){
		return (
			<form className="AutoGainOption">
			<input type="checkbox" onClick={this.handleSubmit} checked={this.props.autogain} ref="autogain" />
			Auto
			</form>
		)
	}
});

var GainOptions = React.createClass({
	handleSubmit: function(){
		var gain=this.refs.gain.getDOMNode().value.trim();
		$.ajax({
			url: '/gain/' + gain,
			dataType: 'json'		
		});
		return false;
	},
	render: function(){
		if (this.props.autogain) {
			this.props.gain=this.props.gains[0];
		}
		var createOption = function(v,i){
			return <option value={v}>{v}</option>;
		}
		return (
			<form className="GainOptions">
			<select value={this.props.gain} ref="gain" onChange={this.handleSubmit}>
			{this.props.gains.map(createOption)}
			</select>
			Gain
			</form>
		)
	}
});

var ModulationOption = React.createClass({
	render: function(){
		return (<form className="ModulationOption">
			<select value={this.props.mod} ref="mod" 
				onChange={this.handleSubmit}
				onBlur={this.handleBlur}
				onFocus={this.handleBlur}>
				<option value="w">WBFM</option>
				<option value="f">FM</option>
				<option value="a">AM</option>
				<option value="l">LSB</option>
				<option value="u">USB</option>
			</select>
			Modulation
			</form>)
	}
});

var dongle_gains=[];

var State = React.createClass({
  refreshData : function(){
  $.ajax({
      url: '/state',
      dataType: 'json',
      success: function(data) {
        this.setState({data: data});
        document.title=data.freq_s;
      }.bind(this),
      error: function(xhr, status, err) {
        console.error("/state", status, err.toString());
      }.bind(this)
    });
  },
  refreshGainList : function(){
  $.ajax({
      url: '/gain/list',
      dataType: 'json',
      success: function(data) {
        dongle_gains=data.gains;
      }.bind(this),
    });
  },
  getInitialState: function() {
    return {data:[]};
  },
  componentDidMount: function() {
    this.interval = setInterval(this.refreshData, 500);
    this.refreshGainList();
  },
  componentWillUnmount: function() {
    clearInterval(this.interval);
  },
  render: function() {
    return (
    	    <div>
    	    <FrequencyDisplay freq={this.state.data.freq_s} />
    	    <SignalMeter signal={this.state.data.s_level} />
    	    <br />
    	    <FrequencyForm freq={this.state.data.freq_s} />
    	    <ModulationOption mod={this.state.data.mod} />
    	    <GainOptions gains={dongle_gains} gain={this.state.data.gain} autogain={this.state.data.autogain} />
    	    <AutoGainEnabled autogain={this.state.data.autogain} />
    	    </div>
    );
  }
});

React.renderComponent(<State />, $("#state")[0]);
